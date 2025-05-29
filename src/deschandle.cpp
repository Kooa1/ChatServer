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
    User user{QSharedPointer<QTcpSocket>(new QTcpSocket)};

    QMutexLocker locker(&queLock);
    {
        if (!user.tcpSocket.data()->setSocketDescriptor(descQue.dequeue())) {
            qWarning() << "desc bind failed : " << user.tcpSocket.data()->errorString();
            user.tcpSocket.data()->disconnectFromHost();
            user.tcpSocket.data()->deleteLater();
            return;
        }
    }

    connect(user.tcpSocket.data(),
            &QTcpSocket::readyRead,
            [this, user = user, sock = user.tcpSocket]() {
                onReadyRead(user, sock);
            });

    connect(user.tcpSocket.data(),
            &QTcpSocket::disconnected,
            [this, sock = user.tcpSocket]() {
                onDisconnect(sock.data()->property("tempId").toInt());
            });
}

void DescHandle::onReadyRead(const User &user, QSharedPointer<QTcpSocket> sock) {
    auto &buffer = user.getBuffer();

    buffer.append(sock.data()->readAll());

    if (buffer.size() < 20) {
        return;
    }

    QDataStream stream(buffer);
    stream.setByteOrder(QDataStream::BigEndian);

    quint32 MAGIC_NUMBER, COMMAND_TYPE, jsonLength;
    quint64 timestamp;

    stream >> MAGIC_NUMBER >> COMMAND_TYPE >> timestamp >> jsonLength;

    qDebug() << "magic" << MAGIC_NUMBER;
    qDebug() << "command" << COMMAND_TYPE;
    qDebug() << "time" << timestamp;
    qDebug() << "len" << jsonLength;

    QJsonObject json = QJsonDocument::fromJson(buffer.mid(20, jsonLength)).object();

    buffer.remove(0, 20 + jsonLength);

    json.insert("tempId", ++tempId);
    if (COMMAND_TYPE == 2) {
        QMutexLocker RPL(&tempLock);
        {
            sock.data()->setProperty("tempId", tempId);
            regPool.insert(tempId, sock);
        }

        QThreadPool::globalInstance()->start(new Register(this, json));

    } else if (COMMAND_TYPE == 1) {
        qDebug() << "login";
        QMutexLocker UPL(&userLock);
        {
            user.tcpSocket.data()->setProperty("tempId", tempId);
            userPool.insert(tempId, user);
        }

        QThreadPool::globalInstance()->start(new Login(this, json));

    } else {

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
        regPool.value(registerResult["tempId"].toInt()).data()->write(data);
        regPool.remove(registerResult["temId"].toInt());
    }
}
