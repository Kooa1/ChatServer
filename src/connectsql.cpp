//
// Created by 66 on 25-5-3.
//

#include "../include/connectsql.h"

ConnectSql::ConnectSql(
        const QString &hostName,
        const QString &userName,
        const QString &dbName,
        const QString &psw,
        const unsigned short int port) {

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(hostName);
    db.setUserName(userName);
    db.setDatabaseName(dbName);
    db.setPassword(psw);
    db.setPort(port);

}

bool ConnectSql::open() {
    if (!db.open()) {
        qDebug() << "mysql conn database failed : " << db.lastError().text();
        return false;
    } else {
        qDebug() << "mysql conn success";
        return true;
    }

}

ConnectSql::~ConnectSql() = default;
