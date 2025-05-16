//
// Created by 66 on 25-5-15.
//

#ifndef CHATSERVER_LOGIN_H
#define CHATSERVER_LOGIN_H

#include <QObject>
#include <QJsonObject>

class Login : public QObject{
Q_OBJECT

public:
    explicit Login(QObject *parent = nullptr);



public slots:
    void recvData(qintptr, const QJsonObject &);

private:
    qintptr descriptor;
    QJsonObject json;

};


#endif //CHATSERVER_LOGIN_H
