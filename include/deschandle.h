//
// Created by 66 on 25-5-20.
//

#ifndef CHATSERVER_DESCHANDLE_H
#define CHATSERVER_DESCHANDLE_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMetaObject>
#include <QQueue>
#include <QThread>

#include "../include/login.h"

class DescHandle : public QObject {
Q_OBJECT

public:
    //构造函数
    explicit DescHandle(QObject *parent = nullptr);

public slots:
    //套接字描述符接收
    void recvDescriptor(qintptr);
    //工作函数
    void working();

    void recvLoginData(const QJsonObject &);
    void recvLoginData(const User &, const QJsonObject &);

private:
    //常驻登陆线程
    QThread *loginThread;
    Login *guardLogin;

    //队列锁
    QMutex queLock;
    //套接字描述服队列
    QQueue<qintptr> descQue;

    //临时id
    qint32 tempId = 1;
    //tcpsocket临时池
    QHash<qint32, QTcpSocket*> tempPool;
    //临时哈希表锁
    QMutex hashLock;

    //用户池锁
    QMutex userLock;
    //认证后的用户池
    QHash<qint32, User> userPool;

private:
    //行为检测
    void actionCheck();
    //初始化常驻登陆线程
    void initLogin();

signals:
    //login工作函数开始工作信号
    void loginStart();
    //数据发送
    void sendLoginData(qint32, const QJsonObject &);

};

#endif //CHATSERVER_DESCHANDLE_H
