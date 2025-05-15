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
    using ResultCallback = std::function<void(const QJsonObject&)>;

    explicit Register(QJsonObject, ResultCallback callback);

    //重载
    void run() override;

private:
    //有参构造用户信息
    QString account;
    QString password;
    QJsonObject json;

    //回调函数返回值
    QJsonObject result;
    //错误信息
    QString errorMsg;
    //回调函数
    ResultCallback m_callback;
private:
    bool acIsExists();

    static QByteArray salt(const int);

    QJsonObject buildJsonMsg(int, const QString &);

    QObject *recvTarget{};
};



#endif //REGISTER_H
