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
    QTcpSocket *tcpSocket = new QTcpSocket();

    QMutexLocker locker(&queLock);{
        if (!tcpSocket->setSocketDescriptor(descQue.dequeue())) {
            qWarning() << "desc bind failed : " << tcpSocket->errorString();
            tcpSocket->disconnectFromHost();
            tcpSocket->deleteLater();
            return;
        }
    }

    connect(tcpSocket, &QTcpSocket::readyRead, [this, tcpSocket](){
        QJsonObject json = QJsonDocument::fromJson(tcpSocket->readAll()).object();
        if (json["action"] == "register") {
            qDebug() << "register";
        } else if (json["action"] == "login") {
            qDebug() << "login";
            tempPool.insert(++tempId, tcpSocket);
            emit loginSendData(tempId, json);
        }
    });

}

void DescHandle::recvDescriptor(qintptr desc) {

    QMutexLocker locker(&queLock);{
        descQue.enqueue(desc);
    }

    QMetaObject::invokeMethod(this->parent(), "startWorker", Qt::QueuedConnection);
}

void DescHandle::initLogin() {
    loginThread = new QThread(this);
    guardLogin = new Login(this);
    guardLogin->moveToThread(loginThread);

    connect(this, &DescHandle::loginSendData, guardLogin, &Login::recvData);
    connect(this, &DescHandle::loginStart, guardLogin, &Login::worker);

    loginThread->start();
}

void DescHandle::recvLoginData() {

}
