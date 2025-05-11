//
// Created by 66 on 25-5-8.
//

#include "../include/registerserver.h"

RegisterServer::RegisterServer(qintptr descriptor, const QJsonObject &json) {
    this->descriptor = descriptor;
    this->json = json;
}


