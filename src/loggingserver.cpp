//
// Created by 66 on 25-5-3.
//

#include "../include/loggingserver.h"


LoggingServer::LoggingServer(QTcpSocket *tcpSocket,QObject *parent) : QObject{parent}, QRunnable() {
    setAutoDelete(true);
    this->tcpSocket = tcpSocket;
}

void LoggingServer::run() {
    qDebug() << tcpSocket;
}

LoggingServer::~LoggingServer()= default;

