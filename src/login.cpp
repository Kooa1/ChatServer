//
// Created by 66 on 25-5-15.
//

#include "../include/login.h"
#include "../include/connectionpool.h"

Login::Login(QObject *object, qint32 tempId, QJsonObject tempJson) : QRunnable() {
    this->object = object;
    this->tempId = tempId;
    this->tempJson = tempJson;

    setAutoDelete(true);
}

void Login::run() {


    if (loginResult(tempId, tempJson)) {
        qDebug() << "success";
    }
}

bool Login::loginResult(qint32 id, const QJsonObject &json) {
    //获取数据库连接
    QSqlDatabase db = ConnectionPool::instance().getConnection();
    //检测有效性
    if (!db.isValid()) {
        qDebug() << "get db error";
        ConnectionPool::instance().releaseConnection(db);
        QMetaObject::invokeMethod(
                object,
                "loginFailed",
                Qt::QueuedConnection,
                Q_ARG(const QJsonObject&, buildJsonMsg(id, -1, QString(db.lastError().text())))
        );
        return false;
    }
    //检测是否开启
    if (!db.open()) {
        qDebug() << "open db error";
        ConnectionPool::instance().releaseConnection(db);
        QMetaObject::invokeMethod(
                object,
                "loginFailed",
                Qt::QueuedConnection,
                Q_ARG(const QJsonObject&, buildJsonMsg(id, -1, QString(db.lastError().text())))
        );
        return false;
    }

    QSqlQuery query(db);
    QString sql = "SELECT uid, phone, password_hash, salt, account_status FROM users";

    if (!query.exec(sql)) {
        qDebug() << "sql exec error";
        ConnectionPool::instance().releaseConnection(db);
        QMetaObject::invokeMethod(
                object,
                "loginFailed",
                Qt::QueuedConnection,
                Q_ARG(const QJsonObject&, buildJsonMsg(id, -1, QString(db.lastError().text())))
        );
        return false;
    }

    //用户数据
    QJsonObject root;
    QJsonArray friendships;
    root["uid"];
    root["friendships"];

    //认证flag
    bool userFound = false;
    //数据库认证
    while (query.next()) {
        if (query.value(1).toString() == json["account"].toString() &&
            query.value(2).toString() == json["password"].toString() + query.value(3).toString()) {
            root["uid"] = query.value(0).toInt();
            userFound = true;
            break;
        }
    }

    if (!userFound) {
        QMetaObject::invokeMethod(
                object,
                "loginFailed",
                Qt::QueuedConnection,
                Q_ARG(const QJsonObject&, buildJsonMsg(id, 0x000, QString(db.lastError().text())))
        );
        return false;
    }

    //好友关系查询
    query.prepare("SELECT * FROM friendships WHERE user1_id = ?");
    query.addBindValue(root["uid"].toInt());
    if (!query.exec()) {
        ConnectionPool::instance().releaseConnection(db);
        QMetaObject::invokeMethod(
                object,
                "loginFailed",
                Qt::QueuedConnection,
                Q_ARG(const QJsonObject&, buildJsonMsg(id, -1, QString(db.lastError().text())))
        );
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

    QMetaObject::invokeMethod(
            object,
            "loginSuccess",
            Qt::QueuedConnection,
            Q_ARG(const QJsonObject&, root),
            Q_ARG(const QJsonObject&, buildJsonMsg(id, 0x001, "Success"))
    );

    ConnectionPool::instance().releaseConnection(db);

    return true;
}

QJsonObject Login::buildJsonMsg(qint32 tempId, qint32 code, const QString &msg) {
    QJsonObject Info{
            {"tempId",    tempId},
            {"code",      code},
            {
             "status",    [](int select) {
                switch (select) {
                    case 0x000:
                        return "Error : login failed";
                    case 0x001:
                        return "Login successful";
                    default:
                        return "internal error";
                }
            }(code)
            },
            {"message",   msg},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };

    return Info;
}

Login::~Login() = default;
