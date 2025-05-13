//
// Created by 66 on 25-5-8.
//

#include "../include/checkaction.h"
#include "../include/Worker.h"

CheckAction::CheckAction(QObject *parent, qint16 port) : QTcpServer(parent){
    this->listen(QHostAddress::Any, port);
    if (this->isListening()) {
        qDebug() << "success";
    }
}

CheckAction::~CheckAction() {
}

void CheckAction::incomingConnection(qintptr descriptor) {
    tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(descriptor);
    tcpList.append(tcpSocket);
}

