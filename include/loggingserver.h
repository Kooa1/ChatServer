//
// Created by 66 on 25-5-3.
//

#ifndef CHATSERVER_LOGGINGSERVER_H
#define CHATSERVER_LOGGINGSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QThreadPool>

class LoggingServer : public QObject, public QRunnable {
    Q_OBJECT

public:
    LoggingServer(qintptr socketDescriptor);

    void run() override;

private:
    qintptr socketDescriptor;
};


#endif //CHATSERVER_LOGGINGSERVER_H
