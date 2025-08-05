//
// Created by 66 on 25-7-28.
//

#include "../include/transmit.h"

Transmit::Transmit() {

}

void Transmit::working(const QJsonObject &json) {
    qDebug() << "start working";

    taskQueue.enqueue(json);

    processTask();
}


void Transmit::processTask() {
    QJsonObject object = taskQueue.dequeue();

    object.remove("action");
    object.insert("action", "receive");

    qint32 recipient = object["recipient"].toVariant().toInt();

    emit taskFinish(recipient, object);
}



