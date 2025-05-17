//
// Created by 66 on 25-5-15.
//

#ifndef CHATSERVER_LOGIN_H
#define CHATSERVER_LOGIN_H

#include <QHash>
#include <QQueue>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QEventLoop>
#include <QTcpSocket>
#include <QJsonObject>
#include <QMutexLocker>

class Login : public QObject{
Q_OBJECT

//结构体保存线程参数
struct ThreadInfo{
    int taskNum;
    QThread *thread;
};

public:
    explicit Login();
    ~Login();

public slots:
    //工作函数
    void worker();
    //数据接收函数
    void recvData(qintptr, const QJsonObject &);

private:
    //数据接收
    QMutex queueMutex;//队列锁
    QQueue<QPair<qintptr, QJsonObject>> taskQueue;
    qintptr descriptor;
    QJsonObject json;

    //活跃用户哈希表
    QHash<qintptr, QTcpSocket*> userPool;

    //线程哈希表
    QHash<int, ThreadInfo> threadPool;
    //分配锁
    QMutex assignMutex;

private:
    //关闭线程
    void destroy();
    //工作分配
    void assignTask(qintptr);

};

#endif //CHATSERVER_LOGIN_H
