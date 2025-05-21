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

class Login : public QObject {
Q_OBJECT

struct tempInfo{
    qint32 tempId;
    QJsonObject json;
};

public:
    explicit Login(QObject *);

    ~Login() override;

public slots:

    //工作函数
    void worker();

    //数据接收函数
    void recvData(qint32, const QJsonObject &);

private:
    //错误信息
    QString errorMsg;

    //上级对象指针
    QObject *object;

    //json队列锁
    QMutex qLock;
    //json数据队列
    QQueue<tempInfo> jsonQueue;


private:
    //数据库验证账号
    bool loginResult(const QJsonObject &);
    //构造json
    QJsonObject buildJsonMsg(int, const QString &);

signals:
    void sendLoginResult();

};

#endif //CHATSERVER_LOGIN_H
