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

#include <QEventLoop>

class Worker : public QObject, public QRunnable {
Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);

    void run() override;

    ~Worker() override;

    void recvInfo(int workType, qintptr descriptor, const QJsonObject&);
private:
    QTcpSocket *tcpSocket;
    qintptr descriptor;
    QJsonObject json;
    int workType;

    //信息构造
    QByteArray errorJson;

signals:

};


#endif //CHATSERVER_REGISTERSERVER_H
