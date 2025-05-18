//
// Created by 66 on 25-5-15.
//

#include "../include/login.h"
#include "../include/connectionpool.h"

Login::Login() {
    //初始化工作线程
    for (int i = 0; i < 3; ++i) {
        QThread *thread = new QThread();
        thread->start();
        ThreadInfo t{0, thread};
        threadPool.insert(i, t);
    }
    qDebug() << "task ready";
}

void Login::worker() {
    QTcpSocket *tcpSocket = new QTcpSocket();
    QPair<qintptr, QJsonObject> taskInfo;

//    QEventLoop loop;
    {
        QMutexLocker locker(&queueMutex);
        if (!taskQueue.isEmpty()) {
            taskInfo = taskQueue.dequeue();
            tcpSocket->setSocketDescriptor(taskInfo.first);
//            connect(tcpSocket, &QTcpSocket::readyRead, this, &Login::onReadyRead);
            connect(tcpSocket, &QTcpSocket::disconnected, this, &Login::onDisconnect);
            userPool.insert(taskInfo.first, tcpSocket);
            qDebug() << "conn ready";

            QJsonObject jsonObject{
                    {"code", 0x001}
            };

            tcpSocket->write(QJsonDocument(jsonObject).toJson());

            connect(tcpSocket, &QTcpSocket::readyRead, this, [=](){
                QString str = tcpSocket->readAll();
                qDebug() << "client : " << str;
                qDebug() << "server : 1";

            });
        }
    }

    if(loginResult(taskInfo.second)){
        qDebug() << "access";
    }
//    loop.exec();
}

//slots
void Login::recvData(qintptr descriptor, const QJsonObject &json) {
    //数据缓冲队列
    QMutexLocker locker(&queueMutex);
//    qDebug() << descriptor;
    taskQueue.enqueue(QPair(descriptor, json));
}

//void Login::assignTask(qintptr) {
//    QMutexLocker locker(&assignMutex);
//
//    int keys;
//    int minLoad = threadPool.value(1).taskNum;
//    QThread *targetThread = nullptr;
//
//    for (QHash<int, ThreadInfo>::iterator it = threadPool.begin(); it != threadPool.end(); ++it) {
//        if (it == threadPool.begin()) continue;
//
//        if (it.value().taskNum < minLoad) {
//            minLoad = it.value().taskNum;
//            targetThread = it.value().thread;
//            keys = it.key();
//        }
//    }
//
//    if (targetThread) {
//        threadPool.value(keys).taskNum + 1;
//        //...
//    }
//
//
//}

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

void Login::destroy() {
    for (QHash<int, ThreadInfo>::iterator it = threadPool.begin(); it != threadPool.end(); ++it) {
        it.value().thread->quit();
        it.value().thread->wait();
    }
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


Login::~Login() {
    destroy();
}

void Login::onReadyRead() {

}

void Login::onDisconnect() {

}
