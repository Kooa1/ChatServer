//
// Created by 66 on 25-5-15.
//

#include "../include/login.h"
#include "../include/connectionpool.h"

Login::Login(QObject *object) {
    this->object = object;
}

void Login::worker() {

}

void Login::recvData(const QJsonObject &json) {

}

bool Login::loginResult(const QJsonObject &json) {
    //获取数据库连接
    QSqlDatabase db = ConnectionPool::instance().getConnection();
    //检测有效性
    if (!db.isValid()) {
        errorMsg = db.lastError().text();
        qDebug() << "get db error";
        ConnectionPool::instance().releaseConnection(db);
        return false;
    }
    //检测是否开启
    if (!db.open()) {
        errorMsg = db.lastError().text();
        qDebug() << "open db error";
        ConnectionPool::instance().releaseConnection(db);
        return false;
    }

    QSqlQuery query(db);
    QString sql = "SELECT phone,password_hash,salt FROM users";
    if (!query.exec(sql)) {
        errorMsg = db.lastError().text();
        qDebug() << "sql exec error";
        ConnectionPool::instance().releaseConnection(db);
        return false;
    }

    while (query.next()) {
        if (query.value(0).toString() == json["account"].toString() &&
            query.value(1).toString() == json["password"].toString() + query.value(2).toString()) {
            ConnectionPool::instance().releaseConnection(db);
            return true;
        } else {
            errorMsg = query.lastError().text();
            return false;
        }
    }

    return false;
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

Login::~Login() = default;
