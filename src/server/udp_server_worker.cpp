#include "udp_server_worker.h"
#include <QDebug>
#include <QMessageBox>

namespace server {

UdpServerWorker::UdpServerWorker(UdpBuffer* buffer, QObject* parent)
    : QObject(parent)
    , buffer_(buffer)
    , device_address_("")
    , device_port_(0)
    , socket_(nullptr)
    , running_(false)
{
    socket_ = new QUdpSocket(this);
    socket_->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 1024);
    socket_->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(socket_, &QUdpSocket::readyRead, this, &UdpServerWorker::read);
}

void UdpServerWorker::start() {
    running_ = true;
    RebindSocket();
}

void UdpServerWorker::stop() {
    running_ = false;
    if (socket_->state() != QAbstractSocket::UnconnectedState) {
        socket_->close();
    }
}

void UdpServerWorker::read() {
    while (running_ && socket_->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket_->pendingDatagramSize());

        QHostAddress sender_addr;
        quint16 sender_port;

        socket_->readDatagram(datagram.data(), datagram.size(), &sender_addr, &sender_port);

        // Фильтруем пакеты от нужного устройства
        if (sender_addr.toString() == device_address_) {
            buffer_->PushPacket(datagram); // Кладем в буфер
        }
    }
}

void UdpServerWorker::Write(const QByteArray& data) {
    socket_->writeDatagram(data, QHostAddress(device_address_), device_port_);
}

void UdpServerWorker::RebindSocket() {
    if (socket_->state() != QAbstractSocket::UnconnectedState) {
        socket_->close();
    }

    if (!socket_->bind(QHostAddress::AnyIPv4, device_port_)) {
        emit errorOccurred("Ошибка привязки сокета: " + socket_->errorString());
    }
}

} // namespace server
