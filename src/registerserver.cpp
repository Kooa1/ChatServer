//
// Created by 66 on 25-5-8.
//

#include "../include/connectionpool.h"
#include "../include/registerserver.h"

RegisterServer::RegisterServer(const qintptr descriptor, const QJsonObject &json, QObject *parent) : QObject(parent), QRunnable() {
    //自动销毁
    setAutoDelete(true);

    this->descriptor = descriptor;
    this->json = json;

    tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(descriptor);

    db = ConnectionPool::instance().getConnection();
    if (!db.isValid()) {
        qDebug() << "get db conn failed, because : " << db.lastError().text();
    }
    if (!db.open()) {
        qDebug() << "open error :" << db.lastError().text();
    }

    this->account = json["account"].toString();
    this->password = json["password"].toString();
}

void RegisterServer::run() {
    if(!accountIsExists(account)) {

    }
}


bool RegisterServer::accountIsExists(QString account) {
    QSqlQuery query(db);
    QString sql = "SELECT phone FROM users";
    if (!query.exec(sql)) {
        qDebug() << query.lastError().text();
    }

    while (query.next()) {
        if (query.value(0).toString() == json["account"].toString()) {
            qDebug() << "account is Exists, is :" << query.value(0).toString();
            return false;
        }
    }
    return true;
}

QByteArray RegisterServer::salt(int length = 32) {
    //创建随机数生成
    QByteArray pwdSalt(length,0);

    QRandomGenerator::system()->fillRange(
        reinterpret_cast<quint32*>(pwdSalt.data()),
        length / sizeof(32));

    qDebug() << pwdSalt;

    return pwdSalt;
}

QByteArray RegisterServer::buildJsonMsg(int code, const QString &msg) {
    QJsonObject json {
        {"code", code},
        {"status", ""},
    };
}


