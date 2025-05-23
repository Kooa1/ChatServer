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

struct User {
    qint32 uid;
    QJsonObject userInfo;
    QTcpSocket *tcpSocket = nullptr;
};

Q_DECLARE_METATYPE(User);

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

    void buildUser();

private:
    //上级对象指针
    QObject *object;

    //json队列锁
    QMutex qLock;
    //json数据队列
    QQueue<QPair<qint32, QJsonObject> > jsonQueue;

    //认证队列锁
    QMutex reLock;
    //认证队列
    QQueue<QPair<QJsonObject, QJsonObject> > resultQueue;

private:
    //数据库验证账号
    bool loginResult(qint32, const QJsonObject &);

    //构造json
    QJsonObject buildJsonMsg(qint32, qint32, const QString &);

signals:
    //发送登陆失败json
    void sendResult(const QJsonObject &);

    //发送登陆成功结构体数据
    void sendResult(const User &, const QJsonObject &);

    //登陆成功构造结构体数据
    void buildStart();
};

#endif //CHATSERVER_LOGIN_H
