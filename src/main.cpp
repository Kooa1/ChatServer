#include <QCoreApplication>
#include <QSqlDatabase>

#include "../include/connectsql.h"
#include "../include/loggingserver.h"

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

    QTcpServer *tcpServer = new QTcpServer;
    if(tcpServer->listen(QHostAddress::Any, 8111))
    {
        qDebug() << "listen failed" << Qt::endl;
    }else{
        qDebug() << "listening" << Qt::endl;
    }

    QObject::connect(tcpServer, &QTcpServer::newConnection, [&](){
        QTcpSocket *tcpSocket = tcpServer->nextPendingConnection();
        QThreadPool::globalInstance()->start(new LoggingServer(tcpSocket));
    });


    return QCoreApplication::exec();

}
