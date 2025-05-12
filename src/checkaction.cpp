//
// Created by 66 on 25-5-8.
//

#include "../include/checkaction.h"
#include "../include/registerserver.h"

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
        //获取tcp通讯套接字描述符
        qintptr descriptor = clientRequest->socketDescriptor();
        // qDebug() << clientRequest;
        //获取客户端行为
        connect(clientRequest, &QTcpSocket::readyRead, [=](){
            // qDebug() << "conn";
            QByteArray data = clientRequest->readAll();
            QJsonObject json = QJsonDocument::fromJson(data).object();
            if(json["action"] == "register"){
                qDebug() << json.value("password");
                //创建任务函数
                RegisterServer *reg = new RegisterServer(descriptor, json);
                //提交线程池
                QThreadPool::globalInstance()->start(reg);
                //将任务提交线程池后销毁socket防止多线程操作套接字--套接字符线程亲和性特征--线程安全性
                tcpServer->close();
                tcpServer->deleteLater();

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
