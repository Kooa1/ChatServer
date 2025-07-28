//
// Created by 66 on 25-7-28.
//

#include "../include/transmit.h"

Transmit::Transmit() {

}

void Transmit::working(const QJsonObject &json) {
    Task task(json);
    qDebug() << "start working";

    qDebug() << task.sender;
    qDebug() << task.recipient;

}
