#include "udp_buffer.h"

namespace server {

UdpBuffer::UdpBuffer(QObject* parent)
    : QObject(parent)
{}

void UdpBuffer::PushPacket(const QByteArray& packet) {
    QMutexLocker locker(&mutex_);

    packets_.enqueue(packet);
    condition_.wakeOne(); // Пробуждаем поток обработки.
}

QVector<QByteArray> UdpBuffer::PopPackets(int maxCount) {
    QMutexLocker locker(&mutex_);

    QVector<QByteArray> result;
    int count = qMin(maxCount, packets_.size());

    for (int i = 0; i < count; ++i) {
        result.append(packets_.dequeue());
    }

    return result;
}

bool UdpBuffer::HasPackets() const {
    QMutexLocker locker(&mutex_);

    return !packets_.isEmpty();
}

} // namespace server

