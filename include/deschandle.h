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
    explicit DescHandle(QObject *parent = nullptr);

public slots:
    void recvDescriptor(qintptr);

    void working();

private:
    qintptr descriptor;

    QThread *loginThread;
    Login *guradLogin;

private:
    void connSort();

    void initLogin();

signals:
    void start();

    void loginStart();
    void loginRecvData(const QJsonObject&);

};


#endif //CHATSERVER_DESCHANDLE_H
