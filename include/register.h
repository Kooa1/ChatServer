//
// Created by 吴文泽 on 25-5-14.
//

#ifndef REGISTER_H
#define REGISTER_H

#include <QDebug>
#include <QObject>
#include <QRunnable>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>


class Register : public QObject, public QRunnable{
Q_OBJECT

public:
    explicit Register(QObject *parent = nullptr);

    void run() override;

private:
    QString account;
    QString password;

    QJsonObject json;

private:
    bool accountIsExists();

    static QByteArray salt(int);

    QByteArray buildJsonMsg(int, const QString &);
};



#endif //REGISTER_H
