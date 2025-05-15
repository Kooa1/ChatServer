//
// Created by 66 on 25-5-11.
//

#include "../include/connectionpool.h"


ConnectionPool::ConnectionPool(QObject *paren) : QObject(paren) {
    QSqlDatabase::addDatabase("QMYSQL");
}

ConnectionPool::~ConnectionPool() {
    destroy();
}

ConnectionPool &ConnectionPool::instance() {
    static ConnectionPool instance;
    return instance;
}

bool ConnectionPool::init(const QString &host, quint16 port, const QString &dbName, const QString &user,
                          const QString &password, int maxConnections) {
    QMutexLocker locker(&mutex);
    this->host = host;
    this->port = port;
    this->dbName = dbName;
    this->user = user;
    this->password = password;
    this->maxConnections = maxConnections;

    for (int i = 0; i < maxConnections / 2; ++i) {
        QSqlDatabase conn = createConnection();
        if(!conn.isOpen()){
            qDebug() << QString("create conn_%1 failed, because : %2").arg(i).arg(conn.lastError().text());
            return false;
        }
        connQueue.enqueue(conn);
    }
    qDebug() << "db pool total" << connQueue.size();
    return true;
}

QSqlDatabase ConnectionPool::getConnection() {
    QMutexLocker locker(&mutex);

    //
    if(!connQueue.isEmpty()){
        // qDebug() << "db get ready";
        return connQueue.dequeue();
    }

    if(currConn < maxConnections){
        QSqlDatabase conn = createConnection();
        if(conn.isOpen()){
            // qDebug() << "new db get ready";
            return conn;
        }
    }

    qDebug() << "Connection Pool Exhausted!";
    return QSqlDatabase();
}

void ConnectionPool::releaseConnection(QSqlDatabase &conn) {
    if(!conn.isValid() || conn.connectionName().isEmpty()){
        return;
    }

    QMutexLocker locker(&mutex);

    //check
    if(conn.isOpen()){
        //执行简单查询验证连接
        QSqlQuery query("select 1", conn);
        if(query.lastError().type() == QSqlError::NoError && query.next()){
            connQueue.enqueue(conn);
            return;
        }
    }

    //失效
    conn.close();
    QSqlDatabase::removeDatabase(conn.connectionName());
    currConn--;
}

void ConnectionPool::destroy() {
    QMutexLocker locker(&mutex);

    while(!connQueue.isEmpty()){
        QSqlDatabase conn = connQueue.dequeue();
        conn.close();
        QSqlDatabase::removeDatabase(conn.connectionName());
    }
    currConn = 0;
}

QSqlDatabase ConnectionPool::createConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL",QString("Connection_%1").arg(++currConn));

    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);

    if(!db.open()){
        qDebug() << "Create connection failed: " << db.lastError().text();
        return QSqlDatabase();
    }

    return db;
}
