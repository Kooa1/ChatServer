//
// Created by 66 on 25-5-8.
//

#ifndef CHATSERVER_CHECKACTION_H
#define CHATSERVER_CHECKACTION_H

#include <QObject>
#include <QRunnable>
#include <QTcpSocket>
#include <QDebug>
#include <QThread>
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
    //注册结果槽函数
    void sendResponse(int id, QJsonObject result);

private:
    QTcpSocket *tcpSocket{};
    //注册数据
    int tcpId = 0;
    QMutex sendMutex;
    QHash<int, TcpInfo> regHash;

    QThread *login;

protected:
    void incomingConnection(qintptr descriptor) override;

signals:
    void sendLoginData(qintptr, const QJsonObject&);
};


#endif //CHATSERVER_CHECKACTION_H
