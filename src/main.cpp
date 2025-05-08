#include <QCoreApplication>
#include <vector>


#include "../include/connectsql.h"
#include "../include/loggingserver.h"
#include "../include/checkaction.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    //数据库连接
    const QString hostName = "8.148.211.115";
    const QString userName = "root";
    const QString dbName = "chat_app";
    const QString psw = "Wwz0530.";
    const unsigned short port = 3306;
    ConnectSql db(hostName,userName,dbName,psw,port);

    //设置线程池最大线程数
    QThreadPool::globalInstance()->setMaxThreadCount(10);

    CheckAction checking;


//    QObject::connect(tcpServer, &QTcpServer::newConnection, [&](){
//        QTcpSocket *clientRequest = tcpServer->nextPendingConnection();
//        qintptr descriptor = clientRequest->socketDescriptor();
//        QObject::connect(clientRequest, &QTcpSocket::readyRead, [&](){
//           QByteArray clientInfo = clientRequest->readAll();
//        });
//    });

    return QCoreApplication::exec();

}
