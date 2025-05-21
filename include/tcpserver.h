//
// Created by 66 on 25-5-8.
//

#ifndef CHATSERVER_TCPSERVER_H
#define CHATSERVER_TCPSERVER_H

#include <QMap>
#include <QMutex>
#include <QDebug>
#include <QThread>
#include <QObject>
#include <QRunnable>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThreadPool>
#include <QJsonObject>
#include <QJsonDocument>

#include "../include/deschandle.h"

class TcpServer : public QTcpServer {
Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr, qint16 port = 8111);

protected:
    void incomingConnection(qintptr descriptor) override;

private:
    DescHandle *handle;

private:
    void initHandle();

signals:
    void newDescriptor(qintptr);
    void startWorker();

};


#endif //CHATSERVER_TCPSERVER_H
