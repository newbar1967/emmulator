#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

#include "settings.h"

namespace cfg {

Settings::Settings(QWidget* parent)
    : view_(std::make_unique<SettingsWindow>(parent))
    , soft_filter_(view_->GetUi()->soft_filter_can_id_obj_wgt,
                   view_->GetUi()->soft_filter_can_id_obj_layout,
                   view_->GetUi()->soft_filter_id_le,
                   view_->GetUi()->soft_filter_header_type_cbox,
                   SOFT_FILTER_MAX_NUM_ID
                   )
    , hard_filter_(view_->GetUi()->hard_filter_can_id_obj_wgt,
                   view_->GetUi()->hard_filter_can_id_obj_layout,
                   view_->GetUi()->hard_filter_id_le,
                   view_->GetUi()->hard_filter_header_type_cbox,
                   HARD_FILTER_MAX_NUM_ID
                   )
    , color_filter_(view_->GetUi()->coloring_filter_can_id_obj_wgt,
                   view_->GetUi()->coloring_filter_can_id_obj_layout,
                   view_->GetUi()->coloring_filter_id_le,
                   view_->GetUi()->coloring_filter_header_type_cbox,
                   SOFT_FILTER_MAX_NUM_ID
                   )
{
    view_->SetActionCallback([this](const QObject* action_object){GUIHandler(action_object);});

    SettingsWindow::ActionName action_name(view_.get());
    action_handlers_ = {
        {action_name.SAVE_BTN, [this]() {Save();}},
        {action_name.CANCEL_BTN, [this]() {Cancel();}},
        {action_name.LOG_DIR_BTN, [this]() {SetLogsDir();}},
        {action_name.SOFT_FILTER_ADD_ID_BTN, [this]() {soft_filter_.AddId();}},
        {action_name.SOFT_FILTER_UPDATE_ID_BTN, [this]() {soft_filter_.UpdateId();}},
        {action_name.SOFT_FILTER_DELETE_ID_ALL_BTN, [this]() {soft_filter_.DeleteAll();}},
        {action_name.SOFT_FILTER_DELETE_ID_SELECT_BTN, [this]() {soft_filter_.DeleteSelectedId();}},
        {action_name.HARD_FILTER_ADD_ID_BTN, [this]() {hard_filter_.AddId();}},
        {action_name.HARD_FILTER_UPDATE_ID_BTN, [this]() {hard_filter_.UpdateId();}},
        {action_name.HARD_FILTER_DELETE_ID_ALL_BTN, [this]() {hard_filter_.DeleteAll();}},
        {action_name.HARD_FILTER_DELETE_ID_SELECT_BTN, [this]() {hard_filter_.DeleteSelectedId();}},
        {action_name.COLORING_FILTER_ADD_ID_BTN, [this]() {color_filter_.AddId();}},
        {action_name.COLORING_FILTER_UPDATE_ID_BTN, [this]() {color_filter_.UpdateId();}},
        {action_name.COLORING_FILTER_DELETE_ID_ALL_BTN, [this]() {color_filter_.DeleteAll();}},
        {action_name.COLORING_FILTER_DELETE_ID_SELECT_BTN, [this]() {color_filter_.DeleteSelectedId();}},
        {action_name.COLORING_FILTER_MODE_CHECK, [this]() {ColoringFilterSetEnabled();}},
    };

    // CAN baud rate.
    can_baud_rate_list_ = {
        "1000",
        "500",
        "250",
        "125",
        "95",
        "83.3",
        "62.5",
        "50",
        "47.6",
        "33.3",
        "20"
    };
    view_->GetUi()->can_baud_rate_cbox->addItems(can_baud_rate_list_);

    // CAN header type.
    can_header_type_list_ = {
        STANDART_HEADER,
        EXTENDED_HEADER
    };
    view_->GetUi()->can_header_type_cbox->addItems(can_header_type_list_);
    view_->GetUi()->soft_filter_header_type_cbox->addItems(can_header_type_list_);
    view_->GetUi()->hard_filter_header_type_cbox->addItems(can_header_type_list_);
    view_->GetUi()->coloring_filter_header_type_cbox->addItems(can_header_type_list_);

    // Звук при приеме CAN-сообщений.
    beep_can_need_list_ = {
       "Выкл",
       "Вкл"
    };
    view_->GetUi()->beep_can_need_cbox->addItems(beep_can_need_list_);

    ;
    // Пауза звука при приеме CAN-сообщений.
    beep_can_pause_list_ = {
        "0",
        "100",
        "200",
        "300",
        "400",
        "500",
        "600",
        "700",
        "800",
        "900",
        "1000"
    };
    view_->GetUi()->beep_can_pause_cbox->addItems(beep_can_pause_list_);
}

// Обработка действий GUI.
void Settings::GUIHandler(const QObject* action_object) {
    const QString objName = action_object->objectName();

    if (action_handlers_.contains(objName)) {
        action_handlers_[objName](); // Вызываем обработчик.
    } else {
        qWarning() << "Unknown action object name:" << objName;
    }
}

void Settings::Update() {
    // Передать всем объектам факт обновления конфигурации.
    const auto callbacks = cbs_;
    for (auto const& cb : callbacks) {
        if (cb != nullptr) {
            cb();
        }
    }
}

// Показать окно настроек.
void Settings::Exec() {
    // Сохраним текущие натройки.
    value_undo_ = value_;
    // Очистим некоторые текстовые поля.
    ClearSomeTextFields();
    // Текущие настройки передаем в представление.
    ModelToUi();
    // Показываем окно.
    view_->exec();
}

void Settings::ClearSomeTextFields() {
    view_->GetUi()->soft_filter_id_le->clear();
    view_->GetUi()->hard_filter_id_le->clear();
}

// Нажатие кнопки "Сохранить".
void Settings::Save() {
    // Преобразование из представления в текущие настройки.
    UiToModel();
    // Передаем настройки всем объектам.
    Update();
    // Закрываем окно.
    view_->hide();
}

// Нажатие кнопки "Отмена".
void Settings::Cancel() {
    // Возвращаем значения настроек.
    value_ = value_undo_;
    // Закрываем окно.
    view_->hide();
}

// Преобразование из представления в текущие настройки.
void Settings::UiToModel() {
    // Директория для лог-файлов.
    SaveLogsDirectory(view_->GetUi()->logs_dir_le->text()); // Эта настройка в системном ключе.

    // CAN baud rate.
    auto baud_rate = view_->GetUi()->can_baud_rate_cbox->currentText();
    value_.can_parameters.baud_rate = baud_rate.toDouble() * BAUD_RATE_COEF;

    // CAN header type.
    value_.can_parameters.header_type = view_->GetUi()->can_header_type_cbox->currentText();

    // Программный фильтр.
    value_.soft_filter_by_id = soft_filter_.FromUiToList();

    // Аппаратный фильтр.
    value_.hard_filter_by_id = hard_filter_.FromUiToList();

    // Подкраска сообщений.
    value_.color_by_id = color_filter_.FromUiToList();

    // Режим подкраски.
    value_.message_coloring_mode = !view_->GetUi()->coloring_mode_check->checkState();

    // Звук при приеме CAN-сообщений.
    value_.hard_settings.beep_can_need = view_->GetUi()->beep_can_need_cbox->currentIndex();

    // Пауза звука при приеме CAN-сообщений.
    auto beep_can_pause = view_->GetUi()->beep_can_pause_cbox->currentText();
    value_.hard_settings.beep_can_pause = beep_can_pause.toUInt();
}

// Текущие настройки передаем в представление.
void Settings::ModelToUi() {
    // Директория для лог-файлов.
    view_->GetUi()->logs_dir_le->setText(LoadLogsDirectory());

    // CAN baud rate
    auto baud_rate = value_.can_parameters.baud_rate / BAUD_RATE_COEF;
    view_->GetUi()->can_baud_rate_cbox->setCurrentText(QString::number(baud_rate));

    // CAN header type
    auto header_type = value_.can_parameters.header_type;
    view_->GetUi()->can_header_type_cbox->setCurrentText(header_type);

    // Программный фильтр.
    soft_filter_.FromListToUi(value_.soft_filter_by_id);

    // Аппаратный фильтр.
    hard_filter_.FromListToUi(value_.hard_filter_by_id);

    // Подкраска сообщений.
    color_filter_.FromListToUi(value_.color_by_id);

    // Режим подкраски.
    view_->GetUi()->coloring_mode_check->setChecked(!value_.message_coloring_mode);
    ColoringFilterSetEnabled();

    // Звук при приеме CAN-сообщений.
    auto beep_need = value_.hard_settings.beep_can_need;
    view_->GetUi()->beep_can_need_cbox->setCurrentIndex(beep_need);

    // Пауза звука при приеме CAN-сообщений.
    QString beep_can_pause = QString::number(value_.hard_settings.beep_can_pause);
    view_->GetUi()->beep_can_pause_cbox->setCurrentText(beep_can_pause);
}

void Settings::SaveCfgDirectory(const QString& dir) {
    SaveDirectory(CFG_DIR_NAME, dir);
}

void Settings::SaveLogAllMsgDirectory(const QString& dir) {
    SaveDirectory(LOG_ALL_MSG_DIR_NAME, dir);
}

void Settings::SaveLogsDirectory(const QString& dir) {
    SaveDirectory(LOGS_DIR_NAME, dir);
}

void Settings::SaveDirectory(const QString& key, const QString& dir) {
    if (!QDir(dir).exists()) {
        qWarning() << "Failed to save directory" << key << "to settings";
        return;
    }

    QSettings settings;
    settings.setValue(key, dir);

    if (settings.status() != QSettings::NoError) {
        qWarning() << "Failed to save directory" << key << "to settings";
    }
}

QString Settings::LoadCfgDirectory() const {
    QSettings settings;
    return settings.value(CFG_DIR_NAME, QDir::homePath()).toString();
}

QString Settings::LoadLogAllMsgDirectory() const {
    QSettings settings;
    return settings.value(LOG_ALL_MSG_DIR_NAME, QDir::homePath()).toString();
}

QString Settings::LoadLogsDirectory() const {
    QSettings settings;
    return settings.value(LOGS_DIR_NAME, QDir::homePath()).toString();
}

// Вызывает диалог выбора директории.
void Settings::SetLogsDir() {
    QString initial_dir; // Какую директорию сделать начальной в диалоге.
    QString tmp_dir = view_->GetUi()->logs_dir_le->text();

    // Проверим, существует ли на самом деле директория из line edit.
    if (QDir(tmp_dir).exists()) {
        initial_dir = tmp_dir; // Директория правильная, начинаем с нее.
    } else {
        // Директория неправильная.
        initial_dir = QDir::homePath(); // Начнем с домашней директории.
    }

    QString logs_dir = QFileDialog::getExistingDirectory(
        QApplication::activeWindow(), // Родительское окно.
        "Выберите папку", // Заголовок диалога.
        initial_dir, // Начальная директория.
        QFileDialog::ShowDirsOnly // Показываем только директории.
        | QFileDialog::DontResolveSymlinks // Не разрешаем символические ссылки.
        );

    if (!logs_dir.isEmpty()) {
        view_->GetUi()->logs_dir_le->setText(logs_dir);
    }
}

void Settings::ColoringFilterSetEnabled() {
    view_->GetUi()->coloring_filter_gbox->setEnabled(!view_->GetUi()->coloring_mode_check->checkState());
}

} // namespace cfg
