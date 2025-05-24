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
#include <QMetaType>
#include <QJsonArray>

class Login : public QObject {
Q_OBJECT

public:
    explicit Login(QObject *);

    ~Login() override;

public slots:

    //工作函数
    void worker();

    //数据接收函数
    void recvData(qint32, const QJsonObject &);

private:
    //上级对象指针
    QObject *object;

    //json队列锁
    QMutex qLock;
    //json数据队列
    QQueue<QPair<qint32, QJsonObject> > jsonQueue;

private:
    //数据库验证账号
    bool loginResult(qint32, const QJsonObject &);

    //构造json
    QJsonObject buildJsonMsg(qint32, qint32, const QString &);

signals:

//    //发送失败数据
//    void sendFailedResult(const QJsonObject &);
//
//    //发送成功数据
//    void sendSuccessResult(const QJsonObject &, const QJsonObject &);

};

#endif //CHATSERVER_LOGIN_H
