//
// Created by 66 on 25-5-15.
//

#ifndef CHATSERVER_LOGIN_H
#define CHATSERVER_LOGIN_H

#include <QHash>
#include <QDebug>
#include <QQueue>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QSqlQuery>
#include <QEventLoop>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QMutexLocker>
#include <QJsonDocument>

class Login : public QObject{
Q_OBJECT

public:
    explicit Login(QObject*);
    ~Login() override;

public slots:
    //工作函数
    void worker();
    //数据接收函数
    void recvData(const QJsonObject &);

private:
    //错误信息
    QString errorMsg;

    //上级对象指针
    QObject *object;

    //json队列锁
    QMutex qLock;
    //json数据队列
    QQueue<QJsonObject> jsonQueue;

private:
    bool loginResult(const QJsonObject &);

    QJsonObject buildJsonMsg(int, const QString &);

signals:

};

#endif //CHATSERVER_LOGIN_H
