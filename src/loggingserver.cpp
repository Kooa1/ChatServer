//
// Created by 66 on 25-5-3.
//

#include "../include/loggingserver.h"


LoggingServer::LoggingServer(QTcpSocket *tcpSocket,QObject *parent) : QObject{parent}, QRunnable() {
    setAutoDelete(true);
    this->tcpSocket = tcpSocket;
}

void LoggingServer::run() {
    connect(tcpSocket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray accountInfo = tcpSocket->readAll();
    });
}

LoggingServer::~LoggingServer()= default;

