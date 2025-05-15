//
// Created by 66 on 25-5-8.
//

#include "../include/checkaction.h"
#include "../include/register.h"

CheckAction::CheckAction(QObject *parent, qint16 port) : QTcpServer(parent){
    this->listen(QHostAddress::Any, port);
    if (this->isListening()) {
        qDebug() << "success";
    }
}

CheckAction::~CheckAction() = default;

void CheckAction::incomingConnection(qintptr descriptor) {
    tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(descriptor);

    TcpInfo node = {descriptor, tcpSocket};

    {
        QMutexLocker locker(&sendMutex);
        tcpMap.insert(++tcpId, node);
    }

    qDebug() << tcpId;

    connect(tcpSocket, &QTcpSocket::readyRead, [&](){
        QByteArray data = tcpSocket->readAll();
        QJsonObject json = QJsonDocument::fromJson(data).object();
        if (json["action"] == "register") {
            QThreadPool::globalInstance()->start(new Register(tcpId, json,
                [this](int resId, const QJsonObject &result) {
                QMetaObject::invokeMethod(
                        this,
                        "sendResponse",
                        Qt::QueuedConnection,
                        Q_ARG(int, resId),
                        Q_ARG(QJsonObject, result)
                );
            }));
        } else {
            qDebug() << "other";
        }
    });
}

void CheckAction::sendResponse(int id, QJsonObject result) {
    QMutexLocker locker(&sendMutex);

    TcpInfo socket = tcpMap.value(id);
    socket.tcp->write("1");
    tcpMap.remove(id);
}

