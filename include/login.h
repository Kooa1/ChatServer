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
#include <QRunnable>
#include <QMetaObject>

class Login : public QRunnable {
public:
    explicit Login(QObject *, qint32, QJsonObject);

    ~Login() override;

    void run() override;

public slots:

private:
    //上级对象指针
    QObject *object;

    qint32 tempId;
    QJsonObject tempJson;

private:
    //数据库验证账号
    bool loginResult(qint32, const QJsonObject &);

    //构造json
    QJsonObject buildJsonMsg(qint32, qint32, const QString &);
};

#endif //CHATSERVER_LOGIN_H
