#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QStatusBar>

#include "view.h"
#include "ui_view.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , windowsTitle_(QApplication::applicationName() + QString(" [v. %1]").arg(QApplication::applicationVersion()))
    , action_cb_(nullptr)
{
    ui_->setupUi(this);

    setWindowTitle(windowsTitle_);

    // Устанавливаем параметры растяжения областей.
    // Области считаются слева направо.
    // в методе setStretchFactor вначале устанавливается индекс
    // области, а потом её вес.
    ui_->splitterMain->setStretchFactor(0, 1); // Вес = 1.
    ui_->splitterMain->setStretchFactor(1, 2); // Вес = 2.

    ToolBarInit();
    StatusBarInit();
    SetupConnections();
}

MainWindow::~MainWindow() {
    delete ui_;
}

void MainWindow::SetupConnections()
{
    connect(ui_->start_msg_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->stop_msg_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->save_all_msg_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->start_record_msg_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->stop_record_msg_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->clear_wnd_msg_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->add_answer_obj_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->hide_answer_obj_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->show_all_answer_obj_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->del_all_answer_obj_btn, &QPushButton::clicked, this, &MainWindow::ActionClicked);
    connect(ui_->mode_cbox, &QComboBox::currentTextChanged, this, &MainWindow::ActionClicked);
}

void MainWindow::ToolBarInit()
{
    ui_->toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    actFileConfigRead_ = ui_->toolBar->addAction(QPixmap("://images/file_read.png"), "Чтение конфигурации из файла", this, &MainWindow::ActionClicked);
    actFileConfigRead_->setObjectName("file_сonfig_read");
    actFileConfigSave_ = ui_->toolBar->addAction(QPixmap("://images/file_save.png"), "Сохранение конфигурации в файл", this, &MainWindow::ActionClicked);
    actFileConfigSave_->setObjectName("file_сonfig_save");
    actSettings_ = ui_->toolBar->addAction(QPixmap("://images/gearwheel.png"), "Открыть окно настроек", this, &MainWindow::ActionClicked);
    actSettings_->setObjectName("settings");

    // Пружинка с разделителем.
    QWidget* spring = new QWidget();
    spring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui_->toolBar->addWidget(spring);
    ui_->toolBar->addSeparator(); // Разделитель перед статус-меткой.

    // Создаём QLabel для текста.
    can_info_lbl_ = new QLabel(this);
    // Добавляем на панель инструментов.
    ui_->toolBar->addWidget(can_info_lbl_);

    // Виджет с константной шириной для отступа.
    QWidget* const_width_wgt = new QWidget();
    const_width_wgt->setFixedWidth(4);
    ui_->toolBar->addWidget(const_width_wgt);
}

void MainWindow::StatusBarInit() {
    statusBar()->addWidget(new QLabel("Ethernet сообщений принято:", this));
    cnt_all_recive_msg_lbl_ = new QLabel(this);
    cnt_all_recive_msg_lbl_->setFixedWidth(60);
    statusBar()->addWidget(cnt_all_recive_msg_lbl_);

    // Создаём рамку-разделитель (вертикальная линия).
    CreateStatusBarSeparator();

    statusBar()->addWidget(new QLabel("CAN сообщений принято:", this));
    cnt_can_recive_msg_lbl_ = new QLabel(this);
    cnt_can_recive_msg_lbl_->setFixedWidth(60);
    statusBar()->addWidget(cnt_can_recive_msg_lbl_);

    // Создаём рамку-разделитель (вертикальная линия).
    CreateStatusBarSeparator();

    record_piсture_lbl_ = new QLabel(this);
    record_piсture_lbl_->setPixmap(QPixmap("://images/record16x16.png"));
    statusBar()->addWidget(record_piсture_lbl_);
    record_text_lbl_ = new QLabel(this);
    statusBar()->addWidget(record_text_lbl_);
    SetVisibleRecordLbl(false);

    statusBar()->addPermanentWidget(new QLabel("Связь с устройством", this));
    connect_status_lbl_ = new QLabel(this);
    SetConnectStatus(false);
    statusBar()->addPermanentWidget(connect_status_lbl_);
}

void MainWindow::CreateStatusBarSeparator() {
    // Создаём рамку-разделитель (вертикальная линия).
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::VLine); // Вертикальная линия.
    separator->setFrameShadow(QFrame::Raised);
    separator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    separator->setFixedWidth(2); // Ширина линии — 2 пикселя.
    statusBar()->addWidget(separator);
}

void MainWindow::ActionClicked() {
    if (!action_cb_) {
        return;
    }

    action_cb_(sender());
}

void MainWindow::SetActionCallback(CallBackAction cb) {
    if (cb != nullptr) { // Проверим указатель.
        action_cb_ = cb;
    }
}

void MainWindow::SetConnectStatus(bool status) {
    if (status) {
        connect_status_lbl_->setPixmap(QPixmap("://images/connect_ok.png"));
    } else {
        connect_status_lbl_->setPixmap(QPixmap("://images/connect_bad.png"));
    }
}

void MainWindow::SetEnabledRecordLbl(bool enabled) {
    record_piсture_lbl_->setEnabled(enabled);
}

void MainWindow::SetVisibleRecordLbl(bool visible) {
    record_text_lbl_->setVisible(visible);
    record_piсture_lbl_->setVisible(visible);
}

void MainWindow::SetTextRecordLbl(const QString& text) {
    record_text_lbl_->setText(text);
}

void MainWindow::SetTextCanInfoLbl(const QString& text) {
    can_info_lbl_->setText(text);
}










