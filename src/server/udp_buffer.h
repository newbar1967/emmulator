#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QByteArray>

namespace server {

class UdpBuffer : public QObject
{
    Q_OBJECT

public:
    explicit UdpBuffer(QObject* parent = nullptr);

    void PushPacket(const QByteArray& packet);
    QVector<QByteArray> PopPackets(int max_count = 100);
    bool HasPackets() const;

private:
    mutable QMutex mutex_;
    QWaitCondition condition_;
    QQueue<QByteArray> packets_;
};

} // namespace server
