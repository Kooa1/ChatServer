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

    void cleanDesc(QTcpSocket*);

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
    void loginRecvData(qintptr, const QJsonObject&);
    void initComplete();

};


#endif //CHATSERVER_DESCHANDLE_H
