#include <QCoreApplication>

#include "../include/tcpserver.h"
#include "../include/connectionpool.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QThreadPool::globalInstance()->setMaxThreadCount(4);

    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<qintptr>("qintptr&");

    qRegisterMetaType<QSharedPointer<QTcpSocket>>("QSharedPointer<QTcpSocket>");


    //数据库连接
    const QString hostName = "8.148.211.115";
    const QString dbName = "chat_app";
    const QString userName = "root";
    const QString pwd = "Wwz0530.";
    quint16 port = 3306;

    //初始化数据库连接池
    ConnectionPool::instance().init(
            hostName,
            port,
            dbName,
            userName,
            pwd
    );

    //监听请求类型
    TcpServer server;

    return QCoreApplication::exec();

}
