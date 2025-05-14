//
// Created by 66 on 25-5-8.
//

#include "../include/checkaction.h"
#include "../include/worker.h"

CheckAction::CheckAction(QObject *parent, qint16 port) : QTcpServer(parent){
    this->listen(QHostAddress::Any, port);
    if (this->isListening()) {
        qDebug() << "success";
    }
}

CheckAction::~CheckAction() = default;

void CheckAction::incomingConnection(qintptr descriptor) {
    tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(descriptor);
    tcpList.append(tcpSocket);
    userPool++;
    connect(tcpSocket, &QTcpSocket::readyRead, [&](){
        QByteArray data = tcpSocket->readAll();
        QJsonObject json = QJsonDocument::fromJson(data).object();
        if (json["action"] == "register") {
            qDebug() << "1";
            QThreadPool::globalInstance()->start(new Worker(descriptor, json));
        }
    });
}

