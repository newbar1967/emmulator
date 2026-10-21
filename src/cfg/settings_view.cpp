#include "settings_view.h"
#include "ui_settings_view.h"

namespace cfg {

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui_(new Ui::SettingsWindow)
{
    ui_->setupUi(this);

    hex_validator_ = CreateHexValidator(); // Создаём валидатор.
    // Для программного фильтра.
    ui_->soft_filter_id_le->setMaxLength(CAN_ID_MAX_LENGTH);
    ui_->soft_filter_id_le->setValidator(hex_validator_);
    // Для аппаратного фильтра.
    ui_->hard_filter_id_le->setMaxLength(CAN_ID_MAX_LENGTH);
    ui_->hard_filter_id_le->setValidator(hex_validator_);
    // Для подкраски.
    ui_->coloring_filter_id_le->setMaxLength(CAN_ID_MAX_LENGTH);
    ui_->coloring_filter_id_le->setValidator(hex_validator_);

    SetupConnections();
}

void SettingsWindow::SetupConnections() {
    connect(ui_->save_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->cancel_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->log_dir_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->soft_filter_add_id_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->soft_filter_delete_id_all_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->soft_filter_delete_id_select_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->hard_filter_add_id_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->hard_filter_delete_id_all_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->hard_filter_delete_id_select_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->coloring_filter_add_id_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->coloring_filter_delete_id_all_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->coloring_filter_delete_id_select_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->coloring_filter_color_set_id_select_btn, &QPushButton::clicked, this, &SettingsWindow::ActionClicked);
    connect(ui_->coloring_mode_check, &QCheckBox::clicked, this, &SettingsWindow::ActionClicked);
}

void SettingsWindow::ActionClicked() {
    if (!action_cb_) {
        return;
    }

    action_cb_(sender());
}

SettingsWindow::~SettingsWindow() {
    delete ui_;
}

void SettingsWindow::SetActionCallback(CallBackAction cb) {
    if (cb != nullptr) { // Проверим указатель.
        action_cb_ = cb;
    }
}

QRegularExpressionValidator* SettingsWindow::CreateHexValidator() {
    QRegularExpression hexRegex("^[0-9A-F]*$");
    return new QRegularExpressionValidator(hexRegex, this);
}

} // namespace cfg

