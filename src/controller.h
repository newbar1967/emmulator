#pragma once

#include <QPlainTextEdit>
#include <QMap>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "view.h"
#include "capture_answer_controller.h"
#include "capture_answer_view.h"
#include "capture_answer_model.h"
#include "settings.h"
#include "config.h"
#include "storage.h"
#include "capture_manager.h"
#include "messages_dispatcher.h"

class QByteArray;

class Controller final : public QObject {
    Q_OBJECT

public:
    Controller() = delete;
    Controller(MainWindow& view,
               MessagesDispatcher& messages_dispather,
               cfg::Settings& settings,
               cfg::Storage& storage,
               cfg::Config& config,
               QObject* parent = nullptr);

    ~Controller();
    Controller(Controller& other) = delete;
    Controller& operator=(Controller& other) = delete;
    Controller(Controller&& other) = delete;
    Controller& operator=(Controller&& other) = delete;

    void UpdateConfig();
    void UpdateSettings();
    void CaptureAnswerDatasToConfig();

private:
    using ActionHandler = std::function<void()>;  // Тип обработчика.

    void StartPrintMessages();
    void StopPrintMessages();
    void StartRecordMessagesToFile();
    void StopRecordMessagesToFile();
    void ClearAllMessages();
    void ModeBoxAction();
    void SettingsWindowShow();
    void CaptureAnswersHide();
    void AllCaptureAnswersShow();
    void AllCaptureAnswersDelete();
    void FileConfigRead();
    void FileConfigSave();
    void GUIHandler(const QObject* action_object);
    void CaptureAnswerDatasFromConfig();
    void AddNewCaptureAnswer();
    void CreateCaptureAnswer(const cfg::CaptureAnswerData& capture_answer_data);
    void CaptureAnswerDelete(emm::CaptureAnswerController* capture_answer_controller);
    void CaptureParametersChanged(emm::CaptureAnswerController* capture_answer_controller);
    void CreateCaptureList();
    void RecordIndicator();
    void SaveAllMessagesToFile();

    QList<emm::CaptureAnswerView*> GetCaptureAnswerViews() {
        return view_.GetUi()->answer_obj_wgt->findChildren<emm::CaptureAnswerView*>();
    }

private:
    static constexpr int TIME_RECORD_INDICATOR = 1000;
    MainWindow& view_;
    MessagesDispatcher& messages_dispather_;
    cfg::Settings& settings_;
    cfg::Storage& storage_;
    cfg::Config& config_;
    bool toggle_record_indicator_;
    QMap<QString, ActionHandler> action_handlers_;  // Карта обработчиков.
    QList<emm::CaptureAnswerController*> capture_answer_controllers_;
    QList<emm::CaptureAnswerModel*> capture_answer_models_;
    emm::CaptureManager capture_manager_;
    QFile log_file_;
    QTextStream log_file_stream_;
    QTimer* timer_record_indicator_;
};
