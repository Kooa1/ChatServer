//
// Created by 吴文泽 on 25-5-14.
//

#include "../include/register.h"
#include "../include/connectionpool.h"

Register::Register(QObject *object, QJsonObject tempJson) : QRunnable() {
    this->object = object;
    this->tempId = tempJson["tempId"].toInt();

    this->account = tempJson["account"].toString();
    this->password = tempJson["password"].toString();

    setAutoDelete(true);
}

void Register::run() {
    qDebug() << "ready";
    qDebug() << "account : " << account << "password : " << password;
    if (acIsExists()) {
        QMetaObject::invokeMethod(
                object,
                "registerHandle",
                Qt::QueuedConnection,
                Q_ARG(const QJsonObject &, buildJsonMsg(0x000, errorMsg))
        );
        return;
    }
    if (!insertInfoDB()) {
        QMetaObject::invokeMethod(
                object,
                "registerHandle",
                Qt::QueuedConnection,
                Q_ARG(const QJsonObject &, buildJsonMsg(0x002, errorMsg))
        );
        return;
    }

    QMetaObject::invokeMethod(
            object,
            "registerHandle",
            Qt::QueuedConnection,
            Q_ARG(const QJsonObject &, buildJsonMsg(0x001, "Success"))
    );

}

//随机盐值
QByteArray Register::salt(int len) {
    if (len <= 0) {
        len = 32;
    }

    QByteArray pwdSalt(len, 0);

    // 使用系统提供的密码学安全随机数
    QRandomGenerator::system()->fillRange(
            reinterpret_cast<quint32 *>(pwdSalt.data()),
            (len + sizeof(quint32) - 1) / sizeof(quint32));

    // 确保返回正确长度
    return pwdSalt.left(len);
}

//检测账号是否存在
bool Register::acIsExists() {
    //获取数据库连接
    QSqlDatabase db = ConnectionPool::instance().getConnection();

    //检测有效性
    if (!db.isValid()) {
        qDebug() << "get db error";
        ConnectionPool::instance().releaseConnection(db);
        errorMsg = db.lastError().text();
        return false;
    }
    //检测是否开启
    if (!db.open()) {
        qDebug() << "open db error";
        ConnectionPool::instance().releaseConnection(db);
        errorMsg = db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    QString sql = "SELECT phone FROM users";
    if (!query.exec(sql)) {
        qDebug() << "sql exec error";
        ConnectionPool::instance().releaseConnection(db);
        errorMsg = query.lastError().text();
        return false;
    }

    while (query.next()) {
        if (query.value(0).toString() == account) {
            ConnectionPool::instance().releaseConnection(db);
            errorMsg = "account Exists";
            return true;
        }
    }
    return false;
}

//添加账号
bool Register::insertInfoDB() {
    //获取数据库连接
    QSqlDatabase db = ConnectionPool::instance().getConnection();

    //检测有效性
    if (!db.isValid()) {
        qDebug() << "get db error";
        ConnectionPool::instance().releaseConnection(db);
        errorMsg = db.lastError().text();
        return false;
    }
    //检测是否开启
    if (!db.open()) {
        qDebug() << "open db error";
        ConnectionPool::instance().releaseConnection(db);
        errorMsg = db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    db.transaction();

    query.prepare("INSERT INTO users (phone, password_hash, salt) VALUE(?, ?, ?)");
    query.addBindValue(account);
    Salt = salt(32);
    query.addBindValue(password + Salt);
    query.addBindValue(Salt);

    if (!query.exec()) {
        qDebug() << "insert error ,account : " << account;
        errorMsg = query.lastError().text();
        db.rollback();
        ConnectionPool::instance().releaseConnection(db);
        return false;
    }
    db.commit();
    ConnectionPool::instance().releaseConnection(db);
    return true;
}

//构造注册返回信息
QJsonObject Register::buildJsonMsg(int code, const QString &msg) {
    QJsonObject Info{
            {"tempId", tempId},
            {"code",      code},
            {"status",    [](int select) {
                switch (select) {
                    case 0x000:
                        return "Error : The account already exists";
                    case 0x001:
                        return "Registration successful";
                    default:
                        return "internal error";
                }
            }(code)},
            {"message",   msg},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };

    return Info;
}
