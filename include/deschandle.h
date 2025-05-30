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
#include <QThreadPool>
#include <utility>
#include <QDataStream>


//用户数据
struct User {
    qint32 uid;
    QJsonObject userInfo;
    QSharedPointer<QTcpSocket> tcpSocket;
    mutable QByteArray buffer;

    User() = default;

    User(QSharedPointer<QTcpSocket> socket)
            : tcpSocket(std::move(socket)) {}
};

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

    void onReadyRead(qint32);

    void onDisconnect(qint32);

    void loginFailed(const QJsonObject &);

    void loginSuccess(const QJsonObject &, const QJsonObject &);

    void registerHandle(const QJsonObject &);

    void taskAssign(qint32, qint32, const QJsonObject &);

private:
    //描述符队列锁
    QMutex queLock;
    //套接字描述服队列
    QQueue<qintptr> descQue;

    //临时id
    qint32 tempId = 0;

    //注册用户池锁
    QMutex tempLock;
    //注册用户临时套接字
    QHash<qint32, QSharedPointer<QTcpSocket>> tempPool;

    //用户池锁
    QMutex userLock;
    //用户池
    QHash<qint32, User> userPool;

    //数据缓冲池;
//    QHash<QSharedPointer<QTcpSocket>, QByteArray> buffer;

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

    //读毕
    void readComplete(qint32, qint32, const QJsonObject &);

};

#endif //CHATSERVER_DESCHANDLE_H
