#pragma once

#include <QObject>
#include <optional>
#include <QByteArray>
#include <QColor>
#include <QTextStream>
#include <unordered_set>
#include "view.h"
#include "udp_server.h"
#include "settings.h"
#include "capture_manager.h"
#include "common_cfg.h"

class MessagesDispatcher final : public QObject {
    Q_OBJECT

    enum class workMode {
        OFF,
        EMMULATOR,
        MONITOR
    };

    struct EthernetMessage {
        qint16 size;
        qint8 status;
        qint8 type;
        QByteArray payload;
    };

    // Константы для разбора Ethernet пакета.
    struct EthernetPacket {
        static constexpr int MIN_SIZE = 5; // Минимальный размер ethernet пакета:
                                           // 2 байта длина, 1 байт ошибка, 1 байт команда, данные, 1 байт cs.
        static constexpr int DATA_BEGIN = 4;
    };

    struct CanMessage {
        quint32 number;
        quint32 time_stamp;
        quint32 id;
        quint32 size;
        QByteArray payload;
    };

    // Константы для разбора CAN пакета.
    struct CanPacket {
        static constexpr int SIZE = 24; // Размер can пакета.
        static constexpr int HEADER_SIZE = 16; // Размер заголовка.
        static constexpr int DATA_BEGIN = 16;
    };

    // Типы сообщений (команды) для связи с устройством.
    enum message_type: quint8 {
        SOFT_RESET = 1,     // Программный сброс устройства.
        GET_ERRORS_LIST,    // Получить список ошибок устройства.
        START_MONITOR,      // Запуск передачи CAN сообщений в ethernet.
        START_EMMULATOR,    // Включает возможность посылки CAN сообщений в шину.
        STOP_CAN_MESSAGES,  // Остановка передачи CAN сообщений в ethernet.
        CAN_DATA,           // Сообщение с CAN данными внутри.
        SEND_CAN_MESSAGE,   // Передать сообщение CAN в шину.
        PING,               // Проверяет связь с устройством.
        SET_HARD_SETTINGS   // Записывает аппаратные настройки устройства во flash-память.
    };

    class ReDraw {
    public:
        ReDraw(MainWindow& view) : view_(view) {
            view_.GetUi()->text_msg->setUpdatesEnabled(false); // Запрет перерисовки.
        }

        ~ReDraw() {
            view_.GetUi()->text_msg->setUpdatesEnabled(true); // Разрешение перерисовки.
        }

    private:
        MainWindow& view_;
    };

public:
    MessagesDispatcher() = delete;
    MessagesDispatcher(MainWindow& view,
               server::UdpServer& udp_server,
               cfg::Settings& settings,
               QObject* parent = nullptr);

    ~MessagesDispatcher();
    MessagesDispatcher(MessagesDispatcher& other) = delete;
    MessagesDispatcher& operator=(MessagesDispatcher& other) = delete;
    MessagesDispatcher(MessagesDispatcher&& other) = delete;
    MessagesDispatcher& operator=(MessagesDispatcher&& other) = delete;

    const QString CanMessageToQString(const CanMessage& can_message) const;
    const QByteArray PrepareMessageToSend(quint8 message_type, const QByteArray& data);
    void ParseCanMessages(const QByteArray& data);
    void SetWorkMode(const QString& mode);
    void UpdateSettings();

    void SetNeedPrint(bool need_print) {
        need_print_ = need_print;
    }

    void SetLogFileStream(QTextStream* log_file_stream) {
        log_file_stream_ = log_file_stream;
    }

    void SetCaptureManager(emm::CaptureManager* capture_manager) {
        capture_manager_ = capture_manager;
    }

private:
    void ParseDeviceMessage(const QByteArray& data);
    bool ParseEthernetMessage(const QByteArray& data);
    std::optional<CanMessage> ParseCanMessage(const QByteArray& data);
    void ReadDeviceMessages(const QVector<QByteArray>& data);
    void SetCountAllReciveMessages(quint32 count);
    void SetCountCanReciveMessages(quint32 count);
    void ClrCountCanReciveMessages();
    std::optional<QColor> ColoringByCanId(quint32 can_id);
    bool FilteringByCanId(quint32 can_id);
    void PrintMessage(const QString& message, QColor color_back = Qt::white);
    void SendMessageCan(quint32 can_id, const QByteArray& data);
    void TimeoutHandling(quint8 message_type);
    void DeleteMessageTypeFromList(quint8 message_type);
    void SendMessageImpl(quint8 message_type,
                         const QByteArray& data,
                         bool response_is_needed,
                         int timeout_ms);
    void StopCanMessagesProcessing();
    void StartMonitorProcessing();
    void StartEmmulatorProcessing();
    void CanDataProcessing(const QByteArray& payload);
    void StopCanMessagesTimeoutHandling();
    void StartMonitorTimeoutHandling();
    void StartEmmulatorTimeoutHandling();
    void PingTimeoutHandling();
    void HardSettingsTimeoutHandling();
    void WorkModeStatusToInitial();
    void PingProcessing(const QByteArray& payload);
    void HardSettingsProcessing();
    QByteArray PrepareDataForStartEmmulatorOrMonitor();
    QByteArray PrepareDataForHardSettings();
    void PrintCanInfo(const cfg::CanParameters& can_parameters);
    void EmmulatorWork(const CanMessage& can_message);

    static constexpr int TIMEOUT_DEFAULT = 500;

    void SendMessage(quint8 message_type) {
        SendMessageImpl(message_type, {}, false, TIMEOUT_DEFAULT);
    }

    void SendMessage(quint8 message_type, const QByteArray& data) {
        SendMessageImpl(message_type, data, false, TIMEOUT_DEFAULT);
    }

    void SendMessage(quint8 message_type, bool response_is_needed) {
        SendMessageImpl(message_type, {}, response_is_needed, TIMEOUT_DEFAULT);
    }

    void SendMessage(quint8 message_type, const QByteArray& data, bool response_is_needed) {
        SendMessageImpl(message_type, data, response_is_needed, TIMEOUT_DEFAULT);
    }

private:
    MainWindow& view_;
    server::UdpServer& udp_server_;
    cfg::Settings& settings_;
    quint32 count_all_recive_messages_;
    quint32 count_can_recive_messages_;
    bool need_print_;
    workMode work_mode_;
    QString previous_mode_string_;
    QTextStream* log_file_stream_;
    bool connect_status_;
    EthernetMessage ethernet_message_;
    std::vector<CanMessage> can_messages_;
    std::unordered_set<quint8> send_message_types_;
    utils::TimeConvert::convertType time_convert_type_;
    cfg::CanParameters can_parameters_;
    cfg::HardSettings hard_settings_;
    cfg::IdWithParameters soft_filter_by_id_;
    cfg::IdWithParameters hard_filter_by_id_;
    cfg::IdWithParameters color_by_id_;
    QString all_can_messages_text_;
    bool message_coloring_mode_;
    emm::CaptureManager* capture_manager_ = nullptr;
};


