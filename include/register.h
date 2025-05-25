//
// Created by 吴文泽 on 25-5-14.
//

#ifndef REGISTER_H
#define REGISTER_H

#include <QDebug>
#include <QObject>
#include <QRunnable>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QEventLoop>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QJsonDocument>
#include <QRandomGenerator>


class Register : public QRunnable {

public:
    explicit Register(QObject *, QJsonObject);

    //重载
    void run() override;

private:
    //上级指针
    QObject *object;
    //临时id
    qint32 tempId;
    //有参构造用户信息
    QString account;
    QString password;

    QString Salt;

    QString errorMsg;

private:
    bool acIsExists();

    static QByteArray salt(const int);

    bool insertInfoDB();

    QJsonObject buildJsonMsg(int, const QString &);

};


#endif //REGISTER_H
