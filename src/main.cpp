#include <QCoreApplication>


//#include "../include/connectsql.h"
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

    //初始化连接池
    ConnectionPool::instance().init(
            hostName,
            port,
            dbName,
            userName,
            pwd
    );
    //监听请求类型
    CheckAction checking;

    return QCoreApplication::exec();

}
