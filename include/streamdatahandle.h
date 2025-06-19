//
// Created by 吴文泽 on 25-5-30.
//

#ifndef STREAMDATAHANDLE_H
#define STREAMDATAHANDLE_H

#include <QRunnable>
#include <QSharedPointer>

class StreamDataHandle : public QRunnable {
    explicit StreamDataHandle();

public slots:
    void recvTcp(QSharedPointer<QTcpSocket>);
};


#endif //STREAMDATAHANDLE_H
