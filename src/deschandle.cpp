//
// Created by 66 on 25-5-20.
//

#include "../include/deschandle.h"

DescHandle::DescHandle(QObject *parent) : QObject(parent) {
    initLogin();
}

void DescHandle::working() {
    qDebug() << "1";
    connSort();

}

void DescHandle::connSort() {
    qDebug() << "1";
    QTcpSocket *tcpSocket = new QTcpSocket(this);
    if (!tcpSocket->setSocketDescriptor(descriptor)) {
        qWarning() << "bind descriptor failed : " << tcpSocket->errorString();
    }

    connect(tcpSocket, &QTcpSocket::readyRead, this, [=]() {
        QJsonObject json = QJsonDocument::fromJson(tcpSocket->readAll()).object();
        if (json["action"] == "register") {
            qDebug() << "regi";
            return;
        } else if (json["action"] == "login") {
            qDebug() << "login";
            return;
        }
    });
}

void DescHandle::recvDescriptor(qintptr desc) {
    this->descriptor = desc;
    qDebug() << descriptor;
    QMetaObject::invokeMethod(this->parent(), "startWorker", Qt::QueuedConnection);
}

void DescHandle::initLogin() {
    loginThread = new QThread(this);
    guradLogin = new Login(this);
    guradLogin->moveToThread(loginThread);
    connect(this, &DescHandle::loginRecvData, guradLogin, &Login::recvData);
    connect(this, &DescHandle::loginStart, guradLogin, &Login::worker);

    loginThread->start();
}
