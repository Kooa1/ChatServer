//
// Created by 66 on 25-5-8.
//

#ifndef CHATSERVER_REGISTERSERVER_H
#define CHATSERVER_REGISTERSERVER_H

#include <QRunnable>
#include <QDebug>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>

class RegisterServer : public QObject, public QRunnable {
Q_OBJECT
public:
    explicit RegisterServer(
            qintptr descriptor,
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

    QString errorMsg;

private:
    bool accountIsExists();

    QByteArray salt(int);

    QByteArray buildJsonMsg(int, const QString &);

    // bool insertDb(QString, QString, )
};


#endif //CHATSERVER_REGISTERSERVER_H
