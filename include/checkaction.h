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
#include <QList>


class CheckAction : public QTcpServer {
Q_OBJECT

public:
    explicit CheckAction(QObject *parent = nullptr, qint16 port = 8111);
    ~CheckAction() override;

    int userPool = 0;
private:
    QTcpSocket *tcpSocket;

protected:
    QList<QTcpSocket*> tcpList;
protected:
    virtual void incomingConnection(qintptr descriptor) override;


};


#endif //CHATSERVER_CHECKACTION_H
