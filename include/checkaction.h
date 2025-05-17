//
// Created by 66 on 25-5-8.
//

#ifndef CHATSERVER_CHECKACTION_H
#define CHATSERVER_CHECKACTION_H

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


class CheckAction : public QTcpServer {
Q_OBJECT

public:
    explicit CheckAction(QObject *parent = nullptr, qint16 port = 8111);
    ~CheckAction() override;

public slots:
    //注册结果槽函数
    void sendResponse(const int id, QJsonObject result);

private:
    QTcpSocket *tcpSocket;
    //注册数据
    int tcpId = 0;
    QMutex sendMutex;
    QHash<const int, QTcpSocket*> regHash;

    QThread *thread;

protected:
    void incomingConnection(qintptr descriptor) override;

signals:
    void startWorker();
    void sendLoginData(qintptr, const QJsonObject&);

};


#endif //CHATSERVER_CHECKACTION_H
