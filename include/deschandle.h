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

struct usrInfo {
    QTcpSocket* tcp;
};

public:
    //构造函数
    explicit DescHandle(QObject *parent = nullptr);

public slots:
    //套接字描述符接收
    void recvDescriptor(qintptr);
    //工作函数
    void working();

    void recvUserData();

private:
    //常驻登陆线程
    QThread *loginThread;
    Login *guradLogin;

    //队列锁
    QMutex queLock;
    //套接字描述服队列
    QQueue<qintptr> descQue;

    //
    QHash<qint32, usrInfo> userPool;

private:
    //行为检测
    void actionCheck();
    //初始化常驻登陆线程
    void initLogin();

signals:
    //login工作函数开始工作信号
    void loginStart();
    //数据发送
    void loginRecvData(const QJsonObject&);

};


#endif //CHATSERVER_DESCHANDLE_H
