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

//结构体保存线程参数
struct ThreadInfo{
    int taskNum;
    QThread *thread;
};

public:
    explicit Login();
    ~Login() override;

public slots:
    //工作函数
    void worker();
    //数据接收函数
    void recvData(qintptr, const QJsonObject &);

    void onReadyRead();

    void onDisconnect();

private:
    //数据接收
    QMutex queueMutex;//队列锁
    QQueue<QPair<qintptr, QJsonObject>> taskQueue;

    //错误信息
    QString errorMsg;

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
//    void assignTask(qintptr);

    bool loginResult(const QJsonObject &);

    static QJsonObject buildJsonMsg(int, const QString &);

signals:
    void initSocketComplete();
};

#endif //CHATSERVER_LOGIN_H
