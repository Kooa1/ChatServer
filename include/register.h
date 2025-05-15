//
// Created by 吴文泽 on 25-5-14.
//

#ifndef REGISTER_H
#define REGISTER_H

#include <QDebug>
#include <QObject>
#include <QRunnable>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QEventLoop>


class Register : public QRunnable{

public:
    //回调函数声明
    using ResultCallback = std::function<void(int tcpId, const QJsonObject&)>;

    explicit Register(int, QJsonObject, ResultCallback callback);

    //重载
    void run() override;

private:
    //有参构造用户信息
    QJsonObject json;
    QString account;
    QString password;
    QString Salt;
    int tcpId;

    //回调函数返回值
    QJsonObject result;
    //错误信息
    QString errorMsg;
    //回调函数
    ResultCallback m_callback;

private:
    bool acIsExists();

    static QByteArray salt(const int);

    bool insertInfoDB();

    static QJsonObject buildJsonMsg(int tcpId, int, const QString &);

};



#endif //REGISTER_H
