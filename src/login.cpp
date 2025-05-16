//
// Created by 66 on 25-5-15.
//

#include "../include/login.h"

#include <QTcpSocket>

Login::Login() {
    for (int i = 0; i < 3; ++i) {
        QThread *thread = new QThread();
        thread->start();
        threadPool.enqueue(thread);
    }

}

void Login::worker() {
    QTcpSocket tcp;
    tcp.setSocketDescriptor(descriptor);
    QEventLoop loop;
    qDebug() << descriptor;
    qDebug() << json["account"].toString();
    tcp.write("1");
    loop.exec();
}

void Login::recvData(qintptr descriptor, const QJsonObject &json) {
    this->descriptor = descriptor;
    this->json = json;
}