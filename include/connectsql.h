//
// Created by 66 on 25-5-3.
//

#ifndef CHATSERVER_CONNECTSQL_H
#define CHATSERVER_CONNECTSQL_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

class ConnectSql : public QObject{
    Q_OBJECT

public:
    //有参构造函数
    ConnectSql(
            const QString& hostName,
            const QString& userName,
            const QString& dbName,
            const QString& psw,
            unsigned short int port
    );
    //析构函数
    ~ConnectSql() override;

private:
    QSqlDatabase db;

};


#endif //CHATSERVER_CONNECTSQL_H
