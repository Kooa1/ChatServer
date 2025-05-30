//
// Created by 66 on 25-5-20.
//

#include "../include/login.h"
#include "../include/register.h"
#include "../include/deschandle.h"

DescHandle::DescHandle(QObject *parent) : QObject(parent) {
    initLogin();
}

void DescHandle::working() {
    actionCheck();
}

void DescHandle::actionCheck() {
    qDebug() << "connSort";
    //智能指针
    QSharedPointer<QTcpSocket> tcpSocket = QSharedPointer<QTcpSocket>(new QTcpSocket);

    QMutexLocker locker(&queLock);
    {
        if (!tcpSocket.data()->setSocketDescriptor(descQue.dequeue())) {
            qWarning() << "desc bind failed : " << tcpSocket.data()->errorString();
            return;
        }
        tcpSocket.data()->setProperty("tempId", ++tempId);
        tempPool.insert(tempId, tcpSocket);

        connect(tcpSocket.data(), &QTcpSocket::readyRead, this, [this, tcpSocket]() {
            onReadyRead(tcpSocket.data()->property("tempId").toInt());
        });
        connect(tcpSocket.data(), &QTcpSocket::disconnected, this, [this, tcpSocket]() {
            onDisconnect(tcpSocket.data()->property("tempId").toInt());
        });

        qDebug() << "complete";
    }
}

void DescHandle::onReadyRead(qint32 tid) {
    qDebug() << tempPool.value(tid);
    QByteArray buffer;
    buffer = tempPool.value(tid).data()->readAll();

    while (true) {
        if (buffer.size() < 20) return;

        QDataStream streamData(buffer);
        streamData.setByteOrder(QDataStream::BigEndian);

        quint32 MAGIC_NUMBER, COMMAND_TYPE, jsonLength;
        quint64 TIMESTAMP;

        streamData >> MAGIC_NUMBER >> COMMAND_TYPE >> TIMESTAMP >> jsonLength;

        qDebug() << "magic" << MAGIC_NUMBER;
        qDebug() << "command" << COMMAND_TYPE;
        qDebug() << "jsonLength" << jsonLength;
        qDebug() << "timestamp" << TIMESTAMP;

        if (MAGIC_NUMBER != 0x4A3B2C1D) {
            qWarning() << "Invalid magic number, closing connection";
            tempPool.value(tid).data()->abort();
            return;
        }

        int totalPacketSize = 20 + jsonLength;
        if (buffer.size() != totalPacketSize) return;

        QByteArray jsonData = buffer.mid(20, jsonLength);
        QJsonObject json = QJsonDocument::fromJson(jsonData).object();

        if (json.isEmpty()) {
            qWarning() << "Invalid JSON data";
            buffer.remove(0, totalPacketSize);
            continue; // 继续处理下一条消息
        }

        buffer.remove(0, totalPacketSize);

        emit readComplete(tid, COMMAND_TYPE, json);

        if (buffer.isEmpty()) {
            break;
        }
    }
}

void DescHandle::taskAssign(qint32 id, qint32 command, const QJsonObject &jsonObject) {
    if (command == 1) {
        if (!tempPool.value(id)->isValid()){
            tempPool.remove(id);
            return;
        }
        User user{QSharedPointer<QTcpSocket> (tempPool.take(id))};
        QThreadPool::globalInstance()->start(new Login(this, jsonObject));
        return;
    }
    if (command == 2) {

    }
    if (command == 3) {

    }
}

void DescHandle::onDisconnect(qint32 poolId) {
    QMutexLocker locker(&userLock);
    {
        userPool.remove(poolId);
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
    qDebug() << resultJson["tempId"].toInt();
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
