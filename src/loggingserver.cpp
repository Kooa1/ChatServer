//
// Created by 66 on 25-5-3.
//

#include "../include/loggingserver.h"


LoggingServer::LoggingServer(qintptr socketDescriptor) : socketDescriptor(socketDescriptor), QRunnable() {
    setAutoDelete(true);
    qDebug() << socketDescriptor;
}

void LoggingServer::run() {

}
