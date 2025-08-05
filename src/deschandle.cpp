//
// Created by 66 on 25-5-20.
//

#include "../include/login.h"
#include "../include/register.h"
#include "../include/deschandle.h"

DescHandle::DescHandle(QObject *parent) : QObject(parent) {

    transferStation = new QThread;
    transmiter = new Transmit;

    transmiter->moveToThread(transferStation);

    connect(this, &DescHandle::start, transmiter, &Transmit::working);
    connect(this, &DescHandle::readComplete, this, &DescHandle::taskAssign);
    connect(transmiter, &Transmit::taskFinish, this, &DescHandle::sendMsg);

    transferStation->start();
}

void DescHandle::working() {
    actionCheck();
}

void DescHandle::actionCheck() {
//    qDebug() << "connSort";
    //智能指针
//    QSharedPointer<QTcpSocket> tcpSocket = QSharedPointer<QTcpSocket>(new QTcpSocket);
    User user{QSharedPointer<QTcpSocket>(new QTcpSocket)};
    QMutexLocker locker(&userLock);
    {
        userPool.insert(++tempId, user);
    }

    QMutexLocker QLK(&queLock);
    {
        if (!user.tcpSocket.data()->setSocketDescriptor(descQue.dequeue())) {
            qWarning() << "desc bind failed : " << user.tcpSocket.data()->errorString();
            return;
        }
        user.tcpSocket.data()->setProperty("tempId", tempId);
    }

    //信号连接
    connect(user.tcpSocket.data(), &QTcpSocket::readyRead, this, [this, user]() {
        onReadyRead(user.tcpSocket.data()->property("tempId").toInt());
    }, Qt::QueuedConnection);
    connect(user.tcpSocket.data(), &QTcpSocket::disconnected, this, [this, user]() {
        onDisconnect(user.tcpSocket.data()->property("tempId").toInt());
    }, Qt::QueuedConnection);

//    qDebug() << "complete" << tempId;
}

void DescHandle::onReadyRead(qint32 tid) {
//    qDebug() << "ready read";
    try {
        QMutexLocker locker(&userLock);

        auto &buffer = userPool[tid].buffer;
        auto &tcp = userPool[tid].tcpSocket;
        buffer.append(tcp.data()->readAll());

        while (true) {
            if (buffer.size() < 20) return;

            QDataStream streamData(buffer);
            streamData.setByteOrder(QDataStream::BigEndian);

            quint32 MAGIC_NUMBER, COMMAND_TYPE, DATALENGTH;
            quint64 TIMESTAMP;

            streamData >> MAGIC_NUMBER >> COMMAND_TYPE >> TIMESTAMP >> DATALENGTH;

//            qDebug() << "magic" << MAGIC_NUMBER;
//            qDebug() << "command" << COMMAND_TYPE;
//            qDebug() << "DATALENGTH" << DATALENGTH;
//            qDebug() << "timestamp" << TIMESTAMP;

            if (MAGIC_NUMBER != 0x4A3B2C1D) {
                qWarning() << "Invalid magic number, closing connection";
                userPool.value(tid).tcpSocket.data()->abort();
                userPool.remove(tid);
                return;
            }

            qint32 totalPacketSize = 20 + DATALENGTH;
            if (buffer.size() < totalPacketSize) return;

            QByteArray packet = buffer.left(totalPacketSize);
            buffer.remove(0, totalPacketSize);

            emit readComplete(COMMAND_TYPE, tid, packet);
        }
    } catch (const std::exception &e) {
        qDebug() << "exception : " << e.what();
    }
}

void DescHandle::taskAssign(qint32 COMMAND_TYPE, qint32 tid, const QByteArray &packet) {
//    qDebug() << "tid" << tid;

    switch (COMMAND_TYPE) {
        //登录注册同一逻辑
        case 0x001:
        case 0x002: {
//            qDebug() << packet;
            QJsonObject json = QJsonDocument::fromJson(packet.mid(20)).object();
            json["tempId"] = tid;
            if (json["action"] == "login") {
                QThreadPool::globalInstance()->start(new Login(this, json));
                qDebug() << "0x001";
            }
            if (json["action"] == "register") {
                QThreadPool::globalInstance()->start(new Register(this, json));
                qDebug() << "0x002";
            }
            break;
        }
            //消息转发
        case 0x003: {
            QJsonObject json = QJsonDocument::fromJson(packet.mid(20)).object();
            qDebug() << "0x003";

            emit start(json);
            break;
        }
        default:
            qWarning() << "Unknown command type:" << COMMAND_TYPE;
            break;
    }

}

void DescHandle::onDisconnect(qint32 poolId) {
    QMutexLocker locker(&userLock);
    {
        if (userPool.contains(poolId)) {
            userPool.remove(poolId);
            qDebug() << "remove :" << poolId;
        }
    }
}

void DescHandle::recvDescriptor(qintptr desc) {
    QMutexLocker locker(&queLock);
    {
        descQue.enqueue(desc);
    }

    QMetaObject::invokeMethod(this->parent(), "startWorker", Qt::QueuedConnection);
}

void DescHandle::loginFailed(const QJsonObject &resultJson) {
    QByteArray data = buildStream(1, resultJson);
    qDebug() << resultJson;

    QMutexLocker locker(&userLock);
    {
        if (userPool.contains(resultJson["tempId"].toInt())) {
            userPool.value(resultJson["tempId"].toInt()).tcpSocket.data()->write(data);
            userPool.remove(resultJson["tempId"].toInt());
        }
    }
}

void DescHandle::loginSuccess(const QJsonObject &rootJson, const QJsonObject &resultJson) {
    qDebug() << "tempid" << resultJson["tempId"].toInt();
    // QByteArray data = QJsonDocument(resultJson).toJson();
    QByteArray data = buildStream(1, resultJson);

    QMutexLocker locker(&userLock);

    User &object = userPool[rootJson["uid"].toInt()];
    object.uid = rootJson["uid"].toInt();
    object.userInfo = rootJson;

    userPool.value(resultJson["tempId"].toInt()).tcpSocket.data()->write(data);

    QMutexLocker sockLock(&tcpLock);
    tcpPool.insert(rootJson["uid"].toInt(), userPool.value(resultJson["tempId"].toInt()));
    qDebug() << object.uid;
}

void DescHandle::registerHandle(const QJsonObject &registerResult) {
    qDebug() << registerResult["tempId"].toInt();
    // QByteArray data = QJsonDocument(registerResult).toJson();
    QByteArray data = buildStream(2, registerResult);

    QMutexLocker locker(&userLock);
    {
        if (userPool.contains(registerResult["tempId"].toInt())) {
            userPool.value(registerResult["tempId"].toInt()).tcpSocket.data()->write(data);
            userPool.remove(registerResult["temId"].toInt());
        }
    }
}

void DescHandle::sendMsg(const qint32 recipientUid, const QJsonObject &json) {
    QMutexLocker locker(&tcpLock);
    qDebug() << "send ready";

    if (!tcpPool.contains(recipientUid)){
        qDebug() << "user is no online";
        return;
    }

    tcpPool.value(recipientUid).tcpSocket.data()->write(buildStream(3, json));
    qDebug() << "send over";
}

QByteArray DescHandle::buildStream(const quint32 COMMAND_TYPE, const QJsonObject &jsonObject) {
    QByteArray data;
    QByteArray jsonData = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
    QDataStream buffer(&data, QIODevice::WriteOnly);
    buffer.setByteOrder(QDataStream::BigEndian);

    const quint32 MAGIC_NUMBER = 0x4A3B2C1D;
    const quint64 TIMESTAMPS = QDateTime::currentMSecsSinceEpoch();

    buffer << MAGIC_NUMBER << COMMAND_TYPE << TIMESTAMPS << jsonData.size();
    data.append(jsonData);

    return data;
}



