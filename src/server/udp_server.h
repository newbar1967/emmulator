#pragma once

#include <QObject>
#include <QThread>
#include "udp_buffer.h"
#include "udp_server_worker.h"
#include "settings.h"

namespace server {

class UdpServer : public QObject
{
    Q_OBJECT

    using CallBackDataReady = std::function<void(const QVector<QByteArray>& data)>;

public:
    UdpServer(cfg::Settings& settings, QObject* parent = nullptr);
    ~UdpServer();
    UdpServer(UdpServer& other) = delete;
    UdpServer& operator=(UdpServer& other) = delete;
    UdpServer& operator=(UdpServer&& other) = delete;
    UdpServer(UdpServer&& other) = delete;

    void SetDataReadyCallback(CallBackDataReady cb);
    void Write(const QByteArray& data);
    void UpdateSettings();

public slots:
    void processBufferedData();
    void showSocketBindError(const QString& error);

private:
    void SetupWorkerThread();

private:
    static constexpr int TIME_PERIODIC_BUFFER_PROCESSING = 25;
    static constexpr int THREAD_COMPLETION_TIMEOUT = 3000;
    static constexpr int NUMBER_OF_PACKETS = 200;
    cfg::Settings& settings_;
    CallBackDataReady data_ready_cb_;
    UdpBuffer buffer_;
    QThread workerThread_;
    UdpServerWorker* worker_;
};

} // namespace server
