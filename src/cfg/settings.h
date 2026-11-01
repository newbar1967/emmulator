#pragma once

#include <QPointer>
#include <QVector>
#include <QList>

#include "settings_view.h"
#include "time_convert.h"
#include "common_cfg.h"
#include "filter.h"
#include "color_filter.h"

namespace cfg {

struct CanParameters {
    QString header_type = STANDART_HEADER;
    quint32 baud_rate = 500'000;
};

struct HardSettings {
    quint32 beep_can_need = 0;  // Индекс.
    quint32 beep_can_pause = 200;
};

class Settings final : public QObject
{
    Q_OBJECT  // Обязательно для работы фильтров событий.

    using CallBack = std::function<void()>;
    using CallBackVector = QVector<CallBack>;

    struct Value {
        QString device_address = "192.168.129.10";
        quint16 device_port = 5000;
        IdWithParameters color_by_id;
        IdWithParameters soft_filter_by_id;
        IdWithParameters hard_filter_by_id;
        CanParameters can_parameters;
        utils::TimeConvert::convertType time_convert_type =
            utils::TimeConvert::convertType::SSSS_MS; // Строковое представление времени сообщения.
        bool message_coloring_mode = true;
        HardSettings hard_settings;
    };

public:
    static constexpr double BAUD_RATE_COEF = 1000.0;

    Settings(QWidget* parent = nullptr);
    Settings(Settings& other) = delete;
    Settings& operator=(Settings& other) = delete;
    Settings(Settings&& other) = delete;
    Settings& operator=(Settings&& other) = delete;

    void SaveCfgDirectory(const QString& dir);
    QString LoadCfgDirectory() const;
    void SaveLogAllMsgDirectory(const QString& dir);
    QString LoadLogAllMsgDirectory() const;
    void SaveLogsDirectory(const QString& dir);
    QString LoadLogsDirectory() const;
    void Exec();
    void Update();

    const QStringList& GetCanBaudRateList() const {
        return can_baud_rate_list_;
    }

    const QString GetCurrentBaudRate() const {
        return view_->GetUi()->can_baud_rate_cbox->currentText();
    }

    const QStringList& GetCanHeaderTypeList() const {
        return can_header_type_list_;
    }

    const QString GetCurrentHeaderType() const {
        return view_->GetUi()->can_header_type_cbox->currentText();
    }

    Value& GetValue() {
        return value_;
    }

    void AddCallBack(CallBack cb) {
        if (cb != nullptr) {
            cbs_.push_back(cb);
        }
    }

    void SetEnabledHardSettingsTab(bool enabled) {
        QWidget *targetPage = view_->GetUi()->hard_settings_tab; // Страница настроек аппаратуры.
        int idx = view_->GetUi()->settings_tab->indexOf(targetPage);
        if (idx != -1) {
            view_->GetUi()->settings_tab->tabBar()->setTabEnabled(idx, enabled);
            targetPage->setEnabled(enabled);
        }
    }

private:
    using ActionHandler = std::function<void()>;  // Тип обработчика.

    void GUIHandler(const QObject* action_object);
    void Save();
    void Cancel();
    void SetLogsDir();
    void SaveDirectory(const QString& key, const QString& dir);
    void ModelToUi();
    void UiToModel();
    void ClearSomeTextFields();
    void ColoringFilterSetEnabled();

private:
    inline static const QString CFG_DIR_NAME = "cfg_dir_name";
    inline static const QString LOG_ALL_MSG_DIR_NAME = "log_all_msg_dir_name";
    inline static const QString LOGS_DIR_NAME = "logs_dir_name";
    static constexpr quint32 SOFT_FILTER_MAX_NUM_ID = 200; // Здесь может быть и больше.
    static constexpr quint32 HARD_FILTER_MAX_NUM_ID = 32; // Ограничено аппаратурой, только 32.
    std::unique_ptr<SettingsWindow> view_;
    Filter soft_filter_;
    Filter hard_filter_;
    ColorFilter color_filter_;
    QMap<QString, ActionHandler> action_handlers_;  // Карта обработчиков.
    CallBackVector cbs_; // Массив функциональных объектов.
    Value value_;
    Value value_undo_; // Для временного хранения настроек.
    QString last_directory_;
    QStringList can_baud_rate_list_;
    QStringList can_header_type_list_;
    QStringList beep_can_need_list_;
    QStringList beep_can_pause_list_;
};

} // namespace cfg
