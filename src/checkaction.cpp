//
// Created by 66 on 25-5-8.
//

#include "../include/checkaction.h"
#include "../include/register.h"
#include "../include/login.h"

CheckAction::CheckAction(QObject *parent, qint16 port) : QTcpServer(parent){
    QThreadPool::globalInstance()->setMaxThreadCount(2);

    login = new QThread();
    Login *worker = new Login;
    worker->moveToThread(login);
    login->start();

    connect(this, &CheckAction::sendLoginData, worker, &Login::recvData);



    this->listen(QHostAddress::Any, port);
    if (this->isListening()) {
        qDebug() << "success";
    }
}

void CheckAction::incomingConnection(qintptr descriptor) {
    tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(descriptor);

    connect(tcpSocket, &QTcpSocket::readyRead, [&](){
        QByteArray data = tcpSocket->readAll();
        QJsonObject json = QJsonDocument::fromJson(data).object();
        if (json["action"] == "register") {
            TcpInfo node = {descriptor, tcpSocket};{
                //共享数据上锁
                QMutexLocker locker(&sendMutex);
                regHash.insert(++tcpId, node);
            }
            //启动线程,调用类内回调函数,invokeMethod向主线程槽函数发送信号
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
        }

        emit sendLoginData(descriptor, json);

    });
}

void CheckAction::sendResponse(int id, QJsonObject result) {
    //共享数据上锁
    QMutexLocker locker(&sendMutex);
    TcpInfo socket = regHash.value(id);
    socket.tcp->write("1");
    regHash.remove(id);
}

CheckAction::~CheckAction() {
    login->wait();
    login->deleteLater();
}