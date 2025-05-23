//
// Created by 66 on 25-5-20.
//

#include "../include/deschandle.h"

DescHandle::DescHandle(QObject *parent) : QObject(parent) {
    initLogin();
}

void DescHandle::working() {
    actionCheck();
}

void DescHandle::actionCheck() {
    qDebug() << "connSort";
    QTcpSocket *tcpSocket = new QTcpSocket();

    QMutexLocker locker(&queLock); {
        if (!tcpSocket->setSocketDescriptor(descQue.dequeue())) {
            qWarning() << "desc bind failed : " << tcpSocket->errorString();
            tcpSocket->disconnectFromHost();
            tcpSocket->deleteLater();
            return;
        }
    }

    connect(tcpSocket, &QTcpSocket::readyRead, [this, tcpSocket]() {
        QJsonObject json = QJsonDocument::fromJson(tcpSocket->readAll()).object();
        if (json["action"] == "register") {
            qDebug() << "register";
        } else if (json["action"] == "login") {
            qDebug() << "login";
            tempPool.insert(++tempId, tcpSocket);
            emit sendLoginData(tempId, json);
        } else {
            qDebug() << "other";
        }
    });
}

void DescHandle::recvDescriptor(qintptr desc) {
    QMutexLocker locker(&queLock);
    {
        descQue.enqueue(desc);
    }

    QMetaObject::invokeMethod(this->parent(), "startWorker", Qt::QueuedConnection);
}

void DescHandle::initLogin() {
    loginThread = new QThread(this);
    guardLogin = new Login(this);
    guardLogin->moveToThread(loginThread);

    connect(this, &DescHandle::sendLoginData, guardLogin, &Login::recvData);
    connect(this, &DescHandle::loginStart, guardLogin, &Login::worker);
    //连接重载信号槽<失败>
    connect(guardLogin, QOverload<const QJsonObject &>::of(&Login::sendResult),
            this, QOverload<const QJsonObject &>::of(&DescHandle::recvLoginData));
    //连接重载信号槽<成功>
    connect(guardLogin, QOverload<const User &, const QJsonObject &>::of(&Login::sendResult),
            this, QOverload<const User &, const QJsonObject &>::of(&DescHandle::recvLoginData));

    loginThread->start();
}

void DescHandle::recvLoginData(const QJsonObject &json) {
    QByteArray data = QJsonDocument(json).toJson();
    qDebug() << json;
    tempPool.value(json["tempId"].toInt())->write(data);
}

void DescHandle::recvLoginData(const User &info, const QJsonObject &json) {

}
