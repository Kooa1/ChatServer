//
// Created by 66 on 25-5-8.
//

#ifndef CHATSERVER_CHECKACTION_H
#define CHATSERVER_CHECKACTION_H

#include <QObject>
#include <QRunnable>
#include <QTcpSocket>
#include <QDebug>
#include <QThreadPool>
#include <QTcpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QMutex>


class CheckAction : public QTcpServer {
Q_OBJECT

struct TcpInfo{
    qintptr descriptor;
    QTcpSocket* tcp;
};

public:
    explicit CheckAction(QObject *parent = nullptr, qint16 port = 8111);
    ~CheckAction() override;

public slots:
    void sendResponse(int id, QJsonObject result);

private:
    QTcpSocket *tcpSocket;

    int tcpId = 0;

    QMap<int, TcpInfo> tcpMap;

    QMutex sendMutex;

protected:
    virtual void incomingConnection(qintptr descriptor) override;
};


#endif //CHATSERVER_CHECKACTION_H
