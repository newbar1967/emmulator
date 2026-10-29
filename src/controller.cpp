#include <QtGlobal>
#include <QByteArray>
#include <QString>
#include <QTextEdit>
#include <QDebug>
#include <QComboBox>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDateTime>
#include <QDir>

#include "controller.h"
//#include "elapsed_time.h"

using namespace utils;
using namespace emm;
using namespace cfg;

Controller::Controller(MainWindow& view,
                       MessagesDispatcher& messages_dispather,
                       Settings& settings,
                       Storage& storage,
                       Config& config,
                       QObject* parent)
    :QObject(parent)
    ,view_(view)
    ,messages_dispather_(messages_dispather)
    ,settings_(settings)
    ,storage_(storage)
    ,config_(config)
    ,toggle_record_indicator_(false)
{
    view_.SetActionCallback([this](const QObject* action_object){GUIHandler(action_object);});

    settings_.AddCallBack([this](){UpdateSettings();});

    config_.AddCallBack([this](){UpdateConfig();});

    messages_dispather_.SetCaptureManager(&capture_manager_);

    MainWindow::ActionName action_name(&view_);
    action_handlers_ = {
        {action_name.START_MSG_BTN, [this]() {StartPrintMessages();}},
        {action_name.STOP_MSG_BTN, [this]() {StopPrintMessages();}},
        {action_name.SAVE_ALL_MSG_BTN, [this]() {SaveAllMessagesToFile();}},
        {action_name.START_RECORD_MSG_BTN, [this]() {StartRecordMessagesToFile();}},
        {action_name.STOP_RECORD_MSG_BTN, [this]() {StopRecordMessagesToFile();}},
        {action_name.CLEAR_WND_MSG_BTN, [this]() {ClearAllMessages();}},
        {action_name.MODE_BOX, [this]() {ModeBoxAction();}},
        {action_name.ADD_ANSWER_OBJ_BTN, [this]() {AddNewCaptureAnswer();}},
        {action_name.HIDE_ANSWER_OBJ_BTN, [this]() {CaptureAnswersHide();}},
        {action_name.SHOW_ALL_ANSWER_OBJ_BTN, [this]() {AllCaptureAnswersShow();}},
        {action_name.DEL_ALL_ANSWER_OBJ_BTN, [this]() {AllCaptureAnswersDelete();}},
        {action_name.ACT_FILE_CONFIG_SAVE, [this]() {FileConfigSave();}},
        {action_name.ACT_FILE_CONFIG_READ, [this]() {FileConfigRead();}},
        {action_name.ACT_SETTINGS, [this]() {SettingsWindowShow();}}
    };

    timer_record_indicator_ = new QTimer(this);
    connect(timer_record_indicator_, &QTimer::timeout, this, &Controller::RecordIndicator);
}

Controller::~Controller() {
    StopRecordMessagesToFile();
    AllCaptureAnswersDelete();
}

// Обработка действий GUI.
void Controller::GUIHandler(const QObject* action_object) {
    const QString object_name = action_object->objectName();

    if (action_handlers_.contains(object_name)) {
        action_handlers_[object_name](); // Вызываем обработчик.
    } else {
        qWarning() << "Unknown action object name: " << object_name;
    }
}

// Показать окно настроек.
void Controller::SettingsWindowShow() {
    settings_.Exec();
}

// Запуск печати сообщений в окне.
void Controller::StartPrintMessages() {
    messages_dispather_.SetNeedPrint(true);
    view_.GetUi()->stop_msg_btn->setEnabled(true);
    view_.GetUi()->start_msg_btn->setEnabled(false);
}

// Остановка печати сообщений в окне.
void Controller::StopPrintMessages() {
    messages_dispather_.SetNeedPrint(false);
    view_.GetUi()->stop_msg_btn->setEnabled(false);
    view_.GetUi()->start_msg_btn->setEnabled(true);
}

// Запись всех сообщений из окна в файл.
void Controller::SaveAllMessagesToFile() {
    QFileDialog file_dialog;

    file_dialog.setDirectory(settings_.LoadLogAllMsgDirectory()); // Вспомним директорию для лог-файлов.

    QString full_file_name = file_dialog.getSaveFileName(QApplication::activeWindow(),
                                                         "Сохранение лог-файла",
                                                         "",
                                                         "*.log");
    if (full_file_name.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(full_file_name);
    QString file_name = fileInfo.fileName();

    QFile file(full_file_name);
    bool saveSuccess = false;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << view_.GetUi()->text_msg->toPlainText();

        if (out.status() == QTextStream::Ok) {
            saveSuccess = true;
        }
    }

    file.close();

    if (!saveSuccess) {
        QMessageBox::critical(QApplication::activeWindow(),
                              "Ошибка",
                              "Ошибка сохранения лог-файла " + file_name);
        return;
    }

    settings_.SaveLogAllMsgDirectory(fileInfo.absolutePath()); // Сохраняем директорию для лог-файлов.

    QMessageBox::information(QApplication::activeWindow(),
                            "Информация",
                            "Лог сохранен в файл " + file_name);
}

// Запуск записи сообщений в лог-файл.
void Controller::StartRecordMessagesToFile() {
    // Если запись уже запущена.
    if (log_file_stream_.device() != nullptr) {
        return; // Просто выходим.
    }

    // Создание или открытие директории.
    QString log_file_name = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".log";
    log_file_.setFileName(settings_.LoadLogsDirectory()
                          + "\\"
                          + log_file_name);
    // Открываем файл в режиме добавления (если файла нет — создаём).
    if (log_file_.open(QIODevice::Append | QIODevice::Text)) {
        log_file_stream_.setDevice(&log_file_);
    } else {
        QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                            "Не удалось открыть лог-файл для записи: " + log_file_.errorString());
        return;
    }

    messages_dispather_.SetLogFileStream(&log_file_stream_);

    view_.GetUi()->stop_record_msg_btn->setEnabled(true);
    view_.GetUi()->start_record_msg_btn->setEnabled(false);

    view_.SetTextRecordLbl("Запись файла " + log_file_name);
    view_.SetVisibleRecordLbl(true);

    timer_record_indicator_->start(TIME_RECORD_INDICATOR);
}

// Остановка записи сообщений в лог-файл.
void Controller::StopRecordMessagesToFile() {
    if (log_file_stream_.device() == nullptr) {
        return;
    }

    log_file_stream_.setDevice(nullptr);
    messages_dispather_.SetLogFileStream(nullptr);

    log_file_.close();

    view_.GetUi()->stop_record_msg_btn->setEnabled(false);
    view_.GetUi()->start_record_msg_btn->setEnabled(true);

    timer_record_indicator_->stop();
    toggle_record_indicator_ = false;
    view_.SetEnabledRecordLbl(true);
    view_.SetVisibleRecordLbl(false);
}

// Стираем все сообщения в окне.
void Controller::ClearAllMessages() {
    view_.GetUi()->text_msg->clear();
}

// Событие переключения режима работы.
void Controller::ModeBoxAction() {
    messages_dispather_.SetWorkMode(view_.GetUi()->mode_cbox->currentText());
}

// Скрыть выбранные объекты захвата и ответа.
void Controller::CaptureAnswersHide() {
    for (auto& capture_answer_controller : capture_answer_controllers_) {
        if (capture_answer_controller->GetSelectCheck()) {
            capture_answer_controller->GetView()->setVisible(false);
        }
    }
}

// Показать все объекты захвата и ответа.
void Controller::AllCaptureAnswersShow() {
    for (auto& capture_answer_controller : capture_answer_controllers_) {
        capture_answer_controller->GetView()->setVisible(true);
    }
}

// Удаление всех объектов захвата и ответа.
void Controller::AllCaptureAnswersDelete() {
    // Очистим список захвата.
    capture_manager_.Clear();
    // controllers
    qDeleteAll(capture_answer_controllers_);
    capture_answer_controllers_.clear();
    // views
    auto answer_captures_views = GetCaptureAnswerViews();
    qDeleteAll(answer_captures_views);
    answer_captures_views.clear();
    // models
    qDeleteAll(capture_answer_models_);
    capture_answer_models_.clear();
}

// Добавить новый объект захвата и ответа.
void Controller::AddNewCaptureAnswer() {
    // Значения полей при создании нового объекта захвата и ответа.
    const CaptureAnswerData capture_answer_data{
            .select = false,

            .comment = "",

            .capture_id = "18DA00FA",
            .capture_data = "",
            .capture_pos = "0",
            .capture_bytes_num = "4",

            .answer_id = "18DAFA00",
            .answer_data = "",
            .answer_random_pos = "3",
            .answer_random_bytes_num = "50",
            .answer_random_check = false,
    };
    CreateCaptureAnswer(capture_answer_data);
}

void Controller::CaptureAnswerDatasFromConfig() {
    // По размеру данных.
    for (const auto& capture_answer_data : config_.capture_answer_datas) {
        // Создаем объект по данным.
        CreateCaptureAnswer(capture_answer_data);
    }
}

void Controller::CreateCaptureAnswer(const CaptureAnswerData& capture_answer_data) {
    auto capture_answer_view = new CaptureAnswerView;
    auto capture_answer_model = new CaptureAnswerModel;
    auto capture_answer_controller = new CaptureAnswerController(capture_answer_view,
                                                                 capture_answer_model);

    capture_answer_controller->SetCaptureAnswerData(capture_answer_data);

    view_.GetUi()->answer_obj_layout->insertWidget(GetCaptureAnswerViews().size(), capture_answer_view);
    capture_answer_models_.push_back(capture_answer_model);
    capture_answer_controllers_.push_back(capture_answer_controller);

    // Настройки визуального представления.
    // Если установлена галка "select", скроем объект.
    if (capture_answer_controller->GetSelectCheck()) {
        capture_answer_controller->GetView()->setVisible(false);
    }

    // Подключение сигналов.
    connect(capture_answer_controller,
            &CaptureAnswerController::deleteObject,
            this,
            &Controller::CaptureAnswerDelete);

    connect(capture_answer_controller,
            &CaptureAnswerController::captureParametersChanged,
            this,
            &Controller::CaptureParametersChanged);
}

void Controller::CaptureAnswerDelete(CaptureAnswerController* capture_answer_controller) {
    if (!capture_answer_controller) {
        QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                              "CaptureAnswerDelete вызван с nullptr");
        return;
    }

    // Удалим контроллер из списка захвата.
    capture_manager_.RemoveObj(capture_answer_controller);

    auto controller_it = std::find(capture_answer_controllers_.begin(),
                                   capture_answer_controllers_.end(), capture_answer_controller);
    if (controller_it == capture_answer_controllers_.end()) {
        QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                              "Контроллер не найден для удаления");
        return;
    }

    // Получаем связанные объекты
    auto controller = *controller_it;
    auto view = controller->GetView();
    auto model = controller->GetModel();

    // Строгая проверка всех указателей — если хоть один некорректен, операция отменяется.
    if (!view || !model) {
        QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                              "Некорректные указатели view или model в контроллере");
        return;
    }

    // Проверяем, что layout существует.
    auto layout = view_.GetUi()->answer_obj_layout;
    if (!layout) {
        QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                              "Layout не найден — невозможно удалить view из интерфейса");
        return;
    }

    // Ищем модель в списке.
    auto model_it = std::find(capture_answer_models_.begin(),
                              capture_answer_models_.end(), model);
    if (model_it == capture_answer_models_.end()) {
        QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                              "Модель не найдена в списке моделей");
        return;
    }

    // Все проверки пройдены — выполняем удаление в строгом порядке.

    // Удаляем view из layout.
    layout->removeWidget(view);
    delete view;

    // Удаляем модель.
    delete *model_it;
    capture_answer_models_.erase(model_it);

    // Удаляем контроллер.
    delete controller;
    capture_answer_controllers_.erase(controller_it);
}

void Controller::CaptureAnswerDatasToConfig() {
    config_.capture_answer_datas.clear();
    for (auto& capture_answer_controller : capture_answer_controllers_) {
        config_.capture_answer_datas.push_back(
            capture_answer_controller->GetCaptureAnswerData());
    }
}

void Controller::FileConfigRead() {
    if (storage_.LoadViaFileDialog()) {
        config_.Update(); // Всем объектам обновить конфигурацию.
        settings_.Update(); // Всем объектам обновить настройки.
    }
}

void Controller::FileConfigSave() {
    CaptureAnswerDatasToConfig();
    storage_.SaveViaFileDialog();
}

// Обновить конфигурационные данные.
void Controller::UpdateConfig() {
    QTimer::singleShot(0, [this]() {
        AllCaptureAnswersDelete();
        CaptureAnswerDatasFromConfig();
        CreateCaptureList();
    });
}

// Обновить настройки.
void Controller::UpdateSettings() {
}

void Controller::CreateCaptureList() {
    capture_manager_.Clear(); // Очистим данные менеджера захвата.

    for (auto& capture_answer_controller : capture_answer_controllers_) {
        auto capture_key = capture_answer_controller->GetModel()->GetCaptureString();
        capture_manager_.Insert(capture_key, capture_answer_controller);
    }
}

void Controller::CaptureParametersChanged(emm::CaptureAnswerController* capture_answer_controller) {
    auto capture_key = capture_answer_controller->GetModel()->GetCaptureString();
    capture_manager_.Insert(capture_key, capture_answer_controller);
}

void Controller::RecordIndicator() {
    if (toggle_record_indicator_ == false) {
        toggle_record_indicator_ = true;
        view_.SetEnabledRecordLbl(false);
    } else {
        toggle_record_indicator_ = false;
        view_.SetEnabledRecordLbl(true);
    }
}













