//
// Created by 吴文泽 on 25-5-14.
//

#include "../include/register.h"
#include "../include/connectionpool.h"

Register::Register(QObject *parent) : QRunnable(){
    setAutoDelete(true);
    this->account = json["account"].toString();
    this->password = json["password"].toString();
}

void Register::run() {

}


//随机盐值
QByteArray Register::salt(const int len = 32) {
    //创建随机数生成
    QByteArray pwdSalt(len,0);

    QRandomGenerator::system()->fillRange(
        reinterpret_cast<quint32*>(pwdSalt.data()),
        len / sizeof(32));

    qDebug() << pwdSalt;

    return pwdSalt;
}

//检测账号是否存在
bool Register::accountIsExists() {
    //获取数据库连接
    QSqlDatabase db = ConnectionPool::instance().getConnection();
    //检测有效性
    if (!db.isValid()) {
        ConnectionPool::instance().releaseConnection(db);
        qDebug() << "get db conn failed, because : " << db.lastError().text();
        return false;
    }
    //检测是否开启
    if (!db.open()) {
        ConnectionPool::instance().releaseConnection(db);
        qDebug() << "open error :" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    QString sql = "SELECT phone FROM users";
    if (!query.exec(sql)) {

    }

    while (query.next()) {
        if (query.value(0).toString() == json["account"].toString()) {
            qDebug() << "account is Exists, is :" << query.value(0).toString();
            ConnectionPool::instance().releaseConnection(db);
            return false;
        }
    }
    ConnectionPool::instance().releaseConnection(db);
    return true;
}


//构造注册返回信息
QByteArray Register::buildJsonMsg(int code, const QString &msg) {
    QJsonObject errorInfo {
                {"code", code},
                {"status", [](int select){
                    switch (select) {
                        case 0x000: return "register error";
                        case 0x001: return "register success";
                        default: return "internal error";
                    }
                }(code)},
                {"message", msg},
                {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };

    return QJsonDocument(json).toJson();
}
