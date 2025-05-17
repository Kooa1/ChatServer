//
// Created by 吴文泽 on 25-5-14.
//

#include "../include/register.h"
#include "../include/connectionpool.h"

Register::Register(const int tcpId, QJsonObject json, ResultCallback callback)
        : QRunnable(), m_callback(std::move(callback)) {
    setAutoDelete(true);
    this->account = json["account"].toString();
    this->password = json["password"].toString();
    this->tcpId = tcpId;
}

void Register::run() {
    qDebug() << "ready";
    qDebug() << "account : " << account << "password : " << password;
    if (acIsExists()) {
        result = buildJsonMsg(tcpId, 0x000, errorMsg);
        m_callback(tcpId, result);
        return;
    }
    if (!acIsExists()) {

        if (!insertInfoDB()) {
            result = buildJsonMsg(tcpId, 0x002, errorMsg);
            m_callback(tcpId, result);
            return;
        }

        result = buildJsonMsg(tcpId, 0x001, errorMsg);
        m_callback(tcpId, result);
        return;
    }
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
    QString sql = "SELECT phone FROM users";
    if (!query.exec(sql)) {
        errorMsg = query.lastError().text();
        qDebug() << "sql exec error";
        ConnectionPool::instance().releaseConnection(db);
        return false;
    }

    while (query.next()) {
        if (query.value(0).toString() == account) {
            qDebug() << "account is Exists, is :" << query.value(0).toString();
            ConnectionPool::instance().releaseConnection(db);
            return true;
        } else {
            errorMsg = query.lastError().text();
            return false;
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
    db.transaction();

    query.prepare("INSERT INTO users (phone, password_hash, salt) VALUE(?, ?, ?)");
    query.addBindValue(account);
    Salt = salt(32);
    query.addBindValue(password + Salt);
    query.addBindValue(Salt);

    if (!query.exec()) {
        qDebug() << "insert error ,account : " << account;
        db.rollback();
        ConnectionPool::instance().releaseConnection(db);
        return false;
    }
    db.commit();
    ConnectionPool::instance().releaseConnection(db);
    return true;
}

//构造注册返回信息
QJsonObject Register::buildJsonMsg(const int tcpId, int code, const QString &msg) {
    QJsonObject Info{
            {"id",        tcpId},
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
