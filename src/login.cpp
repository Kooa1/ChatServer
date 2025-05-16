//
// Created by 66 on 25-5-15.
//

#include "../include/login.h"

Login::Login(QObject *parent) {

}

void Login::recvData(qintptr descriptor, const QJsonObject &json) {
    this->descriptor = descriptor;
    this->json = json;
}
