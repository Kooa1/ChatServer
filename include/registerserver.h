//
// Created by 66 on 25-5-8.
//

#ifndef CHATSERVER_REGISTERSERVER_H
#define CHATSERVER_REGISTERSERVER_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QJsonObject>

class RegisterServer : public QObject {
Q_OBJECT

public:
    explicit RegisterServer(
            qintptr descriptor,
            const QJsonObject &json
    );

private:
    qintptr descriptor;
    QJsonObject json;
};


#endif //CHATSERVER_REGISTERSERVER_H
