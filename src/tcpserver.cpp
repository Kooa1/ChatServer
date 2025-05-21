//
// Created by 66 on 25-5-8.
//

#include "../include/login.h"
#include "../include/register.h"
#include "../include/tcpserver.h"

TcpServer::TcpServer(QObject *parent, qint16 port) : QTcpServer(parent) {

    initHandle();

    if(!this->listen(QHostAddress::Any, port)){
        qDebug() << this->errorString();
    }

}

void TcpServer::incomingConnection(qintptr descriptor) {
    emit newDescriptor(descriptor);
}

void TcpServer::initHandle() {
    handle = new DescHandle(this);

    connect(this, &TcpServer::newDescriptor, handle, &DescHandle::recvDescriptor);
    connect(this, &TcpServer::startWorker, handle, &DescHandle::working);
}