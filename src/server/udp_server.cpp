#include "udp_server.h"
#include <QTimer>
#include <QMessageBox>

namespace server {

UdpServer::UdpServer(cfg::Settings& settings, QObject* parent)
    : QObject(parent)
    , settings_(settings)
    , data_ready_cb_(nullptr)
    , worker_(nullptr)
{
    settings_.AddCallBack([this](){UpdateSettings();});

    SetupWorkerThread();

    // Таймер для периодической обработки буфера.
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &UdpServer::processBufferedData);
    timer->start(TIME_PERIODIC_BUFFER_PROCESSING);
}

UdpServer::~UdpServer() {
    if (worker_) {
        // Асинхронно вызываем stop() в потоке worker_.
        QMetaObject::invokeMethod(worker_, &UdpServerWorker::stop, Qt::QueuedConnection);

        // Завершаем поток и ждём его завершения.
        workerThread_.quit();
        if (!workerThread_.wait(THREAD_COMPLETION_TIMEOUT)) { // Таймаут.
            qWarning() << "Worker thread did not finish in time, terminating...";
            workerThread_.terminate();
            workerThread_.wait(); // Ждём принудительного завершения.
        }
    }}

void UdpServer::SetupWorkerThread() {
    worker_ = new UdpServerWorker(&buffer_, nullptr);
    worker_->moveToThread(&workerThread_);

    connect(&workerThread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(&workerThread_, &QThread::started, worker_, &UdpServerWorker::start);
    connect(this, &UdpServer::destroyed, worker_, &UdpServerWorker::stop);
    connect(worker_, &UdpServerWorker::errorOccurred, this, &UdpServer::showSocketBindError);

    workerThread_.start();
}

void UdpServer::SetDataReadyCallback(CallBackDataReady cb) {
    if (cb != nullptr) { // Проверим указатель.
        data_ready_cb_ = cb;
    }
}

void UdpServer::Write(const QByteArray& data)
{
    QMetaObject::invokeMethod(worker_, [this, data]() {
        worker_->Write(data);},
         Qt::QueuedConnection
    );
}

void UdpServer::UpdateSettings()
{
    QMetaObject::invokeMethod(worker_, [this]() {
        // Обновим настройки.
        worker_->SetDeviceAddress(settings_.GetValue().device_address);
        worker_->SetDevicePort(settings_.GetValue().device_port);
        // Перепривяжем сокет.
        worker_->RebindSocket();},
        Qt::QueuedConnection
    );
}

void UdpServer::processBufferedData()
{
    if (buffer_.HasPackets() && data_ready_cb_) {
        auto packets = buffer_.PopPackets(NUMBER_OF_PACKETS);
        data_ready_cb_(packets);
    }
}

void UdpServer::showSocketBindError(const QString& error) {
    QMessageBox::critical(QApplication::activeWindow(), "Ошибка", error);
}

} // namespace server
