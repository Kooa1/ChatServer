#include <QCoreApplication>


//#include "../include/connectsql.h"
#include "../include/loggingserver.h"
#include "../include/checkaction.h"
#include "../include/connectionpool.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    //数据库连接
    const QString hostName = "8.148.211.115";
    const QString dbName = "chat_app";
    const QString userName = "root";
    const QString pwd = "Wwz0530.";
    quint16 port = 3306;
//    ConnectSql db(hostName,userName,dbName,psw,port);

//    if(!db.open()){
//        qDebug() << "0";
//    }else{
//        qDebug() << "1";
//    }

    ConnectionPool::instance().init(
            hostName,
            port,
            dbName,
            userName,
            pwd
    );

    QSqlDatabase db  = ConnectionPool::instance().getConnection();
    if (!db.isValid()){
        qDebug() << "Failed to get database connection!";
    }else{
        qDebug() << "get ready";
    }

    //设置线程池最大线程数
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    //监听请求类型
    CheckAction checking;

    ConnectionPool::instance().destroy();

    return QCoreApplication::exec();

}
