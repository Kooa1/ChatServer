//
// Created by 66 on 25-5-15.
//

#include "../include/login.h"
#include "../include/connectionpool.h"

Login::Login(QObject *object) {
    this->object = object;

}

void Login::worker() {
    QPair<qint32, QJsonObject> tempInfo;

    QMutexLocker locker(&qLock);
    {
        if (!jsonQueue.isEmpty()) {
            tempInfo = jsonQueue.dequeue();
        }
    }

    if (loginResult(tempInfo.first, tempInfo.second)) {
        qDebug() << "success";
    }
}

void Login::recvData(qint32 tempId, const QJsonObject &json) {
    QMutexLocker locker(&qLock);
    {
        jsonQueue.enqueue(QPair(tempId, json));
    }
    QMetaObject::invokeMethod(object, "loginStart", Qt::QueuedConnection);
}

bool Login::loginResult(qint32 tempId, const QJsonObject &json) {
    //获取数据库连接
    QSqlDatabase db = ConnectionPool::instance().getConnection();
    //检测有效性
    if (!db.isValid()) {
        qDebug() << "get db error";
        ConnectionPool::instance().releaseConnection(db);
        buildJsonMsg(-1, QString(db.lastError().text()));
        return false;
    }
    //检测是否开启
    if (!db.open()) {
        qDebug() << "open db error";
        ConnectionPool::instance().releaseConnection(db);
        buildJsonMsg(-1, QString(db.lastError().text()));
        return false;
    }

    QSqlQuery query(db);
    QString sql = "SELECT uid, phone, password_hash, salt, account_status FROM users";

    if (!query.exec(sql)) {
        qDebug() << "sql exec error";
        ConnectionPool::instance().releaseConnection(db);
        buildJsonMsg(-1, QString(db.lastError().text()));
        return false;
    }

    //用户数据
    QJsonObject root;
    QJsonArray friendships;
    root["tempId"] = tempId;
    root["uid"];
    root["friendships"];

    bool userFound = false;
    while (query.next()) {
        if (query.value(1).toString() == json["account"].toString() &&
            query.value(2).toString() == json["password"].toString() + query.value(3).toString()) {
            root["uid"] = query.value(0).toInt();
            userFound = true;
            break;
        }
    }

    if (!userFound){
        buildJsonMsg(0x000, "account doesn't exist");
        return false;
    }

    query.prepare("SELECT * FROM friendships WHERE user1_id = ?");
    query.addBindValue(root["uid"].toInt());
    if (!query.exec()) {
        ConnectionPool::instance().releaseConnection(db);
        buildJsonMsg(-1, QString(db.lastError().text()));
        return false;
    }

    while (query.next()) {
        QJsonArray data;
        for (int i = 0; i < 4; ++i) {
            data.append(query.value(i).toString());
        }
        friendships.append(data);
    }
    root.insert("friendships", friendships);
    qDebug() << buildJsonMsg(0x001, "loginSuccess");

    return true;
}

QJsonObject Login::buildJsonMsg(int code, const QString &msg) {
    QJsonObject Info{
            {"code",      code},
            {"status",    [](int select) {
                switch (select) {
                    case 0x000:
                        return "Error : login failed";
                    case 0x001:
                        return "Login successful";
                    default:
                        return "internal error";
                }
            }(code)},
            {"message",   msg},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };

    return Info;
}

User Login::buildInfo() {

    return User();
}

Login::~Login() = default;
