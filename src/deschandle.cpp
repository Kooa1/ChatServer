//
// Created by 66 on 25-5-20.
//

#include "../include/login.h"
#include "../include/register.h"
#include "../include/deschandle.h"

DescHandle::DescHandle(QObject *parent) : QObject(parent) {
    initLogin();

    connect(this, &DescHandle::readComplete, this, &DescHandle::taskAssign);
}

void DescHandle::working() {
    actionCheck();
}

void DescHandle::actionCheck() {
    qDebug() << "connSort";
    //智能指针
    QSharedPointer<QTcpSocket> tcpSocket = QSharedPointer<QTcpSocket>(new QTcpSocket);

    QMutexLocker QLK(&queLock);
    {
        if (!tcpSocket.data()->setSocketDescriptor(descQue.dequeue())) {
            qWarning() << "desc bind failed : " << tcpSocket.data()->errorString();
            return;
        }
        tcpSocket.data()->setProperty("tempId", ++tempId);
        tempPool.insert(tempId, tcpSocket);
    }

    QMutexLocker BLK(&bufferLock);
    {
        bufferPool.insert(tempId, QByteArray());
    }

    //信号连接
    connect(tcpSocket.data(), &QTcpSocket::readyRead, this, [this, tcpSocket]() {
        onReadyRead(tcpSocket.data()->property("tempId").toInt());
    });
    connect(tcpSocket.data(), &QTcpSocket::disconnected, this, [this, tcpSocket]() {
        onDisconnect(tcpSocket.data()->property("tempId").toInt());
    });

    qDebug() << "complete" << tempId;
}

void DescHandle::onReadyRead(qint32 tid) {
    qDebug() << "ready read";
    QMutexLocker BFL(&bufferLock);
    QByteArray &buffer = bufferPool[tid];

    buffer.append(tempPool.value(tid)->readAll());

    while (true) {
        if (buffer.size() < 20) return;

        QDataStream streamData(buffer);
        streamData.setByteOrder(QDataStream::BigEndian);

        quint32 MAGIC_NUMBER, COMMAND_TYPE, DATALENGTH; quint64 TIMESTAMP;

        streamData >> MAGIC_NUMBER >> COMMAND_TYPE >> TIMESTAMP >> DATALENGTH;

        qDebug() << "magic" << MAGIC_NUMBER;
        qDebug() << "command" << COMMAND_TYPE;
        qDebug() << "DATALENGTH" << DATALENGTH;
        qDebug() << "timestamp" << TIMESTAMP;

        if (MAGIC_NUMBER != 0x4A3B2C1D) {
            qWarning() << "Invalid magic number, closing connection";
            tempPool.value(tid).data()->abort();
            tempPool.remove(tid);
            bufferPool.remove(tid);
            return;
        }

        qint32 totalPacketSize = 20 + DATALENGTH;
        if (buffer.size() < totalPacketSize) return;

        QByteArray packet = buffer.left(totalPacketSize);
        buffer.remove(0, totalPacketSize);

        emit readComplete(COMMAND_TYPE, tid, packet);
    }
}

void DescHandle::taskAssign(qint32 COMMAND_TYPE, qint32 tid, const QByteArray &packet) {
    qDebug() << "tid" << tid;
    try {
        switch (COMMAND_TYPE) {
            //登录注册同一逻辑
            case 0x001:
            case 0x002: {
                QByteArray jsonData = packet.mid(20);
                QJsonObject json = QJsonDocument::fromJson(jsonData).object();
                json["tempId"] = tid;

                User user{QSharedPointer<QTcpSocket>(tempPool.value(tid))};
                userPool.insert(tid, user);
                bufferPool.remove(tid);

                QThreadPool::globalInstance()->start(new Login(this, json));
                return;
            }
                //头像上传
            case 0x003: {
                return;
            }
            default:
                qWarning() << "Unknown command type:" << COMMAND_TYPE;
                return;
        }
    } catch (const std::exception &e){
        qCritical() << e.what();
    }
}

void DescHandle::onDisconnect(qint32 poolId) {
    QMutexLocker locker(&userLock);
    {
        userPool.remove(poolId);
        qDebug() << "remove :" << poolId;
    }
}

void DescHandle::recvDescriptor(qintptr desc) {
    QMutexLocker locker(&queLock);
    {
        descQue.enqueue(desc);
    }

    QMetaObject::invokeMethod(this->parent(), "startWorker", Qt::QueuedConnection);
}

void DescHandle::initLogin() {
}

void DescHandle::loginFailed(const QJsonObject &resultJson) {
    QByteArray data = QJsonDocument(resultJson).toJson();

    QMutexLocker locker(&userLock);
    {
        userPool.value(resultJson["tempId"].toInt()).tcpSocket.data()->write(data);
        userPool.remove(resultJson["tempId"].toInt());
    }
}

void DescHandle::loginSuccess(const QJsonObject &rootJson, const QJsonObject &resultJson) {
    qDebug() << "tempid" << resultJson["tempId"].toInt();
    QByteArray data = QJsonDocument(resultJson).toJson();

    qDebug() << rootJson;
    QMutexLocker locker(&userLock);
    {
        User &object = userPool[rootJson["uid"].toInt()];
        object.uid = rootJson["uid"].toInt();
        object.userInfo = rootJson;

        userPool.value(resultJson["tempId"].toInt()).tcpSocket.data()->write(data);
    }
}

void DescHandle::registerHandle(const QJsonObject &registerResult) {
    qDebug() << registerResult["tempId"].toInt();
    QByteArray data = QJsonDocument(registerResult).toJson();

    QMutexLocker locker(&tempLock);
    {
        tempPool.value(registerResult["tempId"].toInt()).data()->write(data);
        tempPool.remove(registerResult["temId"].toInt());
    }
}
