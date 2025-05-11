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

class CheckAction : public QObject {
Q_OBJECT

public:
    CheckAction();
    ~CheckAction() override;

private:
    QTcpServer *tcpServer;
};


#endif //CHATSERVER_CHECKACTION_H
