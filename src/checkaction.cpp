//
// Created by 66 on 25-5-8.
//

#include "../include/checkaction.h"

CheckAction::CheckAction() {

    tcpServer = new QTcpServer();
    if(!tcpServer->listen(QHostAddress::Any, 8111))
    {
        qDebug() << "listen failed" << Qt::endl;
    }else{
        qDebug() << "listening" << Qt::endl;
    }
    connect(tcpServer, &QTcpServer::newConnection, this, [this](){
        QTcpSocket *clientRequest = tcpServer->nextPendingConnection();
        qintptr descriptor = clientRequest->socketDescriptor();
        qDebug() << clientRequest;

        connect(clientRequest, &QTcpSocket::readyRead, [=](){
            qDebug() << "conn";
            QByteArray data = clientRequest->readAll();
            QJsonObject json = QJsonDocument::fromJson(data).object();
            if(json["action"] == "register"){
                qDebug() << json.value("password");
                qDebug() << "reg :" << descriptor;
            }else if(json["action"] == "login"){
                qDebug() << json.value("password");
                qDebug() << "log : " << descriptor;
            }else{
                qWarning() << &QTcpSocket::errorOccurred << Qt::endl;
                clientRequest->close();
                clientRequest->deleteLater();
            }
        });
    });
}

CheckAction::~CheckAction() {
    tcpServer->deleteLater();
}
