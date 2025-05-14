//
// Created by 66 on 25-5-8.
//

#include "../include/connectionpool.h"
#include "../include/worker.h"

Worker::Worker(QObject *parent) : QObject(parent), QRunnable() {

    //自动销毁
    setAutoDelete(true);

    tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(descriptor);
    qDebug() << "soc :" << descriptor;
}

void Worker::run() {

    // QEventLoop loop;
    if (workType == 1) {

    }
    // loop.exec();
}

Worker::~Worker() {
}

void Worker::recvInfo(int workType ,qintptr descriptor, const QJsonObject &) {
    this->workType = workType;
    this->descriptor = descriptor;
    this->json = json;
};
