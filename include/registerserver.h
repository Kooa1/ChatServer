//
// Created by 66 on 25-5-8.
//

#ifndef CHATSERVER_REGISTERSERVER_H
#define CHATSERVER_REGISTERSERVER_H

#include <QRunnable>
#include <QDebug>
#include <QTcpSocket>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlQuery>

class RegisterServer : public QObject, public QRunnable {
Q_OBJECT
public:
    explicit RegisterServer(
            const qintptr descriptor,
            const QJsonObject &json,
            QObject *parent = nullptr
    );

    void run() override;

    // ~RegisterServer();

private:
    QTcpSocket *tcpSocket;

    qintptr descriptor;
    QJsonObject json;

    QSqlDatabase db;

    QString account;
    QString password;

private:
    bool accountIsExists(QString account);

    QByteArray salt(int);

    QByteArray buildJsonMsg(int, const QString &);

    // bool insertDb(QString, QString, )
};


#endif //CHATSERVER_REGISTERSERVER_H
