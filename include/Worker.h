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
#include <QEventLoop>

class Worker : public QObject, public QRunnable {
Q_OBJECT
public:
    explicit Worker(
            qintptr descriptor,
            const QJsonObject&,
            QObject *parent = nullptr
    );

    void run() override;

    ~Worker() override;

private:
    // QTcpSocket *tcpSocket;

    qintptr descriptor;

    QJsonObject json;

    QSqlDatabase db;

    QString account;
    QString password;

    //信息构造
    QByteArray errorJson;
private:
    bool accountIsExists();

    static QByteArray salt(int);

    QByteArray buildJsonMsg(int, const QString &);

    // bool insertDb(QString, QString, )
};


#endif //CHATSERVER_REGISTERSERVER_H
