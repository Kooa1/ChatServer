//
// Created by 66 on 25-5-11.
//

#ifndef CHATSERVER_CONNECTIONPOOL_H
#define CHATSERVER_CONNECTIONPOOL_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>

class ConnectionPool : public QObject {
Q_OBJECT

public:
    //单例模式
    static ConnectionPool &instance();

    bool init(const QString &host, quint16 port,
              const QString &dbName, const QString &user,
              const QString &password, int maxConnections = 10);

    QSqlDatabase getConnection();

    void releaseConnection(QSqlDatabase &conn);

    void destroy();

private:
    //单例模式-私有构造
    explicit ConnectionPool(QObject *paren = nullptr);
    ~ConnectionPool();

private:

    QSqlDatabase createConnection();
    QQueue<QSqlDatabase> connQueue;
    QMutex mutex;
    QString host;
    quint16 port;
    QString dbName;
    QString user;
    QString password;

    int maxConnections;
    int currConn = 0;
};


#endif //CHATSERVER_CONNECTIONPOOL_H
