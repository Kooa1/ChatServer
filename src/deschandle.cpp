//
// Created by 66 on 25-5-20.
//

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


    connect(user.tcpSocket.data(), &QTcpSocket::readyRead, [this, user]() {
        QJsonObject json = QJsonDocument::fromJson(user.tcpSocket.data()->readAll()).object();
        if (json["action"] == "register") {
            qDebug() << "register";
        } else if (json["action"] == "login") {
            qDebug() << "login";
            QMutexLocker UPL(&userLock);
            user.tcpSocket.data()->setProperty("tempId", ++tempId);
            userPool.insert(tempId, user);

            QThreadPool::globalInstance()->start(new Login(this, tempId, json));
        } else {
            qDebug() << "other";
        }
    });

//    connect(user.tcpSocket.data(),
//            &QTcpSocket::disconnected,
//            [this, &sock = user.tcpSocket]() {
//                onDisconnect(sock);
//    });
}

void DescHandle::onReadyread(QSharedPointer<QTcpSocket>) {

}

void DescHandle::onDisconnect(QSharedPointer<QTcpSocket> &sock) {

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
    QByteArray data = QJsonDocument(resultJson).toJson();
    QMutexLocker locker(&userLock);
    {
        User &object = userPool[rootJson["uid"].toInt()];
        object.uid = rootJson["uid"].toInt();
        object.userInfo = rootJson;

        userPool.value(resultJson["tempId"].toInt()).tcpSocket.data()->write(data);
    }
}


