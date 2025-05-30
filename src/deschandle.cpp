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

    QMutexLocker locker(&queLock); {
        if (!tcpSocket.data()->setSocketDescriptor(descQue.dequeue())) {
            qWarning() << "desc bind failed : " << tcpSocket.data()->errorString();
            return;
        }
        tcpSocket.data()->setProperty("tempId", ++tempId);

        connect(tcpSocket.data(), &QTcpSocket::readyRead, this, [this, tcpSocket]() {
            onReadyRead(tcpSocket.data()->property("tempId").toInt());
        });
        connect(tcpSocket.data(), &QTcpSocket::disconnected, this, [this, tcpSocket]() {
            onDisconnect(tcpSocket.data()->property("tempId").toInt());
        });
        tempPool.insert(tempId, tcpSocket);
    }
}

void DescHandle::onReadyRead(qint32 tid) {

}

void DescHandle::onDisconnect(qint32 poolId) {
    QMutexLocker locker(&userLock); {
        userPool.remove(poolId);
    }
}

void DescHandle::recvDescriptor(qintptr desc) {
    QMutexLocker locker(&queLock); {
        descQue.enqueue(desc);
    }

    QMetaObject::invokeMethod(this->parent(), "startWorker", Qt::QueuedConnection);
}

void DescHandle::initLogin() {
}

void DescHandle::loginFailed(const QJsonObject &resultJson) {
    QByteArray data = QJsonDocument(resultJson).toJson();

    QMutexLocker locker(&userLock); {
        userPool.value(resultJson["tempId"].toInt()).tcpSocket.data()->write(data);
        userPool.remove(resultJson["tempId"].toInt());
    }
}

void DescHandle::loginSuccess(const QJsonObject &rootJson, const QJsonObject &resultJson) {
    qDebug() << resultJson["tempId"].toInt();
    QByteArray data = QJsonDocument(resultJson).toJson();

    qDebug() << rootJson;
    QMutexLocker locker(&userLock); {
        User &object = userPool[rootJson["uid"].toInt()];
        object.uid = rootJson["uid"].toInt();
        object.userInfo = rootJson;

        userPool.value(resultJson["tempId"].toInt()).tcpSocket.data()->write(data);
    }
}

void DescHandle::registerHandle(const QJsonObject &registerResult) {
    qDebug() << registerResult["tempId"].toInt();
    QByteArray data = QJsonDocument(registerResult).toJson();

    QMutexLocker locker(&tempLock); {
        tempPool.value(registerResult["tempId"].toInt()).data()->write(data);
        tempPool.remove(registerResult["temId"].toInt());
    }
}
