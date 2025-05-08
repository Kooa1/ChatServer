//
// Created by 66 on 25-5-8.
//

#include "../include/checkaction.h"

CheckAction::CheckAction() {

    QTcpServer *tcpServer = new QTcpServer();
    if(!tcpServer->listen(QHostAddress::Any, 8111))
    {
        qDebug() << "listen failed" << Qt::endl;
    }else{
        qDebug() << "listening" << Qt::endl;
    }
    connect(tcpServer, &QTcpServer::newConnection, [=](){
        QTcpSocket *clientRequest = tcpServer->nextPendingConnection();
        qintptr descriptor = clientRequest->socketDescriptor();
        qDebug() << clientRequest;

        connect(clientRequest, &QTcpSocket::readyRead, [=](){
            QByteArray clientInfo = clientRequest->readAll();
//            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(clientInfo);
            QJsonObject json = doc.object();
            if (json["action"] == "login"){
                qDebug() << "2";
            }
        });
    });
}
