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
        qDebug() << json;
        if (json["action"] == "register") {
            qDebug() << "register";
        } else if (json["action"] == "login") {
            qDebug() << "login";
            emit loginRecvData(json);
        }
    });


}

void DescHandle::recvDescriptor(qintptr desc) {

    QMutexLocker locker(&queLock);{
        descQue.enqueue(desc);
    }

    QMetaObject::invokeMethod(this->parent(), "startWorker", Qt::QueuedConnection);
}

void DescHandle::recvUserData() {

}

void DescHandle::initLogin() {
    loginThread = new QThread(this);
    guradLogin = new Login(this);
    guradLogin->moveToThread(loginThread);
    connect(this, &DescHandle::loginRecvData, guradLogin, &Login::recvData);
    connect(this, &DescHandle::loginStart, guradLogin, &Login::worker);

    loginThread->start();
}
