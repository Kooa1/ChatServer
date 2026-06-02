#include <QCoreApplication>
#include <QException>

#include "../include/tcpserver.h"
#include "../include/connectionpool.h"
#include "../include/deschandle.h"

int main(int argc, char *argv[]) {
    try {
        QCoreApplication a(argc, argv);

        QThreadPool::globalInstance()->setMaxThreadCount(4);

        qRegisterMetaType<qintptr>("qintptr");
        qRegisterMetaType<qintptr>("qintptr&");

        qRegisterMetaType<QSharedPointer<QTcpSocket>>("QSharedPointer<QTcpSocket>");
        qRegisterMetaType<QSharedPointer<QTcpSocket>>();

        qRegisterMetaType<User>("User");
        qRegisterMetaType<Task>("Task");

        //数据库连接 - 请修改为实际配置
        const QString hostName = "your_host";
        const QString dbName = "your_database";
        const QString userName = "your_username";
        const QString pwd = "your_password";
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

    } catch (const std::exception &e) {
        qDebug() << "Std exception : " << e.what();
        return -1;
//    }
    } catch (const QException &e) {
        qDebug() << "Qt exception : " << e.what();
    }
}
