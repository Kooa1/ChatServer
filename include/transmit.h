//
// Created by 66 on 25-7-28.
//

#ifndef CHATSERVER_TRANSMIT_H
#define CHATSERVER_TRANSMIT_H

#include <QObject>
#include <QQueue>
#include <QQueue>
#include <QDateTime>
#include <QJsonObject>

struct Task{
//            {"action", "send"},
//            {"sender", UtilityTools::getUid()},
//            {"senderName", UtilityTools::getName()},
//            {"recipient", uid},
//            {"msgType", 0},
//            {"content", content},
//            {"outgoing", 1},
//            {"sendTimeStamp", QDateTime::currentSecsSinceEpoch()}
    QString action = "recipient";
    QString sender;
    QString senderName;
    QString recipient;
    qint32 msgType;
    QString content;
    qint32 outgoing;
    qint64 sendTimeStamp;

    Task() = default;

    explicit Task(const QJsonObject &json) {
        action = json["action"].toString();
        sender = json["sender"].toString();
        senderName = json["senderName"].toString();
        recipient = json["recipient"].toString();
        msgType = json["msgType"].toInt();
        content = json["content"].toString();
        outgoing = json["outgoing"].toInt();
        sendTimeStamp = json["sendTimeStamp"].toVariant().toLongLong();
    }
};

class Transmit : public QObject{
Q_OBJECT

public:
    explicit Transmit();

public slots:
    void working(const QJsonObject &);

private:
    QQueue<Task> taskQueue;
};


#endif //CHATSERVER_TRANSMIT_H
