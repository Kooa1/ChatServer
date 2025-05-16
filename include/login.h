//
// Created by 66 on 25-5-15.
//

#ifndef CHATSERVER_LOGIN_H
#define CHATSERVER_LOGIN_H

#include <QObject>
#include <QJsonObject>
#include <QQueue>
#include <QThread>
#include <QEventLoop>

class Login : public QObject{
Q_OBJECT

public:
    explicit Login();

public slots:
    void worker();

    void recvData(qintptr, const QJsonObject &);

private:
    qintptr descriptor;
    QJsonObject json;

private:
    QQueue<QThread*> threadPool;



};


#endif //CHATSERVER_LOGIN_H
