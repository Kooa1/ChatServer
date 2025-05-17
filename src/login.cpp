//
// Created by 66 on 25-5-15.
//

#include "../include/login.h"

Login::Login() {
    //初始化工作线程
    for (int i = 0; i < 3; ++i) {
        QThread *thread = new QThread();
        thread->start();
        ThreadInfo t{0, thread};
        threadPool.insert(i, t);
    }

}

void Login::worker() {
    QTcpSocket *tcpSocket = new QTcpSocket();
    QPair<qintptr, QJsonObject> taskInfo;

    QEventLoop loop;
    {
        QMutexLocker locker(&queueMutex);
        if (!taskQueue.isEmpty()) {
            taskInfo = taskQueue.dequeue();
        }
    }

    tcpSocket->setSocketDescriptor(taskInfo.first);

    loop.exec();
}


//slots
void Login::recvData(qintptr descriptor, const QJsonObject &json) {
    //数据缓冲队列
    QMutexLocker locker(&queueMutex);
    taskQueue.enqueue(QPair(descriptor, json));
}

void Login::destroy() {
    for (QHash<int, ThreadInfo>::iterator it = threadPool.begin(); it != threadPool.end(); ++it) {
        it.value().thread->quit();
        it.value().thread->wait();
    }
}

void Login::assignTask(qintptr) {
    QMutexLocker locker(&assignMutex);

    int keys;
    int minLoad = threadPool.value(1).taskNum;
    QThread *targetThread = nullptr;

    for (QHash<int, ThreadInfo>::iterator it = threadPool.begin(); it != threadPool.end(); ++it) {
        if (it == threadPool.begin()) continue;

        if (it.value().taskNum < minLoad) {
            minLoad = it.value().taskNum;
            targetThread = it.value().thread;
            keys = it.key();
        }
    }

    if (targetThread) {
        threadPool.value(keys).taskNum + 1;
        //...
    }


}

Login::~Login() {
    destroy();
}



