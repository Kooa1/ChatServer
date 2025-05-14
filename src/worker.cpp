//
// Created by 66 on 25-5-8.
//

#include "../include/connectionpool.h"
#include "../include/worker.h"

Worker::Worker(const qintptr descriptor,const QJsonObject &json, QObject *parent) : QObject(parent), QRunnable() {
    //自动销毁
    setAutoDelete(true);

    this->descriptor = descriptor;
    tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(descriptor);

    //获取数据库连接
    db = ConnectionPool::instance().getConnection();
    //检测有效性
    if (!db.isValid()) {
        ConnectionPool::instance().releaseConnection(db);
        qDebug() << "get db conn failed, because : " << db.lastError().text();
        return;
    }
    //检测是否开启
    if (!db.open()) {
        ConnectionPool::instance().releaseConnection(db);
        qDebug() << "open error :" << db.lastError().text();
        return;
    }

    this->account = json["account"].toString();
    this->password = json["password"].toString();

    qDebug() << "ready";
}

void Worker::run() {

    QEventLoop loop;


    connect(tcpSocket, &QTcpSocket::connected, [=]() {
        tcpSocket->write("0x000");
        tcpSocket->waitForBytesWritten();
    });

    loop.exec();
}

//检测账号是否存在
bool Worker::accountIsExists() {
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
    return true;
}

//随机盐值
QByteArray Worker::salt(const int len = 32) {
    //创建随机数生成
    QByteArray pwdSalt(len,0);

    QRandomGenerator::system()->fillRange(
        reinterpret_cast<quint32*>(pwdSalt.data()),
        len / sizeof(32));

    qDebug() << pwdSalt;

    return pwdSalt;
}

//构造注册返回信息
QByteArray Worker::buildJsonMsg(int code, const QString &msg) {
    QJsonObject errorInfo {
            {"code", code},
            {"client", descriptor},
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

Worker::~Worker() {
};