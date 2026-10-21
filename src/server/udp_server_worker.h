#pragma once

#include <QObject>
#include <QUdpSocket>
#include "udp_buffer.h"
#include "settings.h"

namespace server {

class UdpServerWorker : public QObject
{
    Q_OBJECT

public:
    explicit UdpServerWorker(UdpBuffer* buffer, QObject* parent = nullptr);

    void Write(const QByteArray& data);
    void RebindSocket();

    void SetDeviceAddress(const QString& device_address) {
        device_address_ = device_address;
    }

    void SetDevicePort(quint16 device_port) {
        device_port_ = device_port;
    }

public slots:
    void start();
    void stop();
    void read();

signals:
    void errorOccurred(const QString& error);

private:
    UdpBuffer* buffer_;
    QString device_address_;
    quint16 device_port_;
    QUdpSocket* socket_;
    bool running_;
};

} // namespace server
