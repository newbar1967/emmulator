#pragma once

#include <QMainWindow>
#include "ui_view.h"

class QPushButton;
class QPlainTextEdit;
class QLabel;
class QComboBox;
class QVBoxLayout;
class QAction;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow final : public QMainWindow
{
    Q_OBJECT

    using CallBackAction = std::function<void(const QObject* qobject_action)>;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void SetActionCallback(CallBackAction cb);
    void SetConnectStatus(bool status);
    void SetEnabledRecordLbl(bool enabled);
    void SetVisibleRecordLbl(bool visible);
    void SetTextRecordLbl(const QString& text);
    void SetTextCanInfoLbl(const QString& text);

    const Ui::MainWindow* GetUi() const {
        return ui_;
    }

    QLabel* CntCanRecMsgLbl() {
        return cnt_can_recive_msg_lbl_;
    }

    QLabel* CntAllRecMsgLbl() {
        return cnt_all_recive_msg_lbl_;
    }

    struct ActionName {
        const QString START_MSG_BTN;
        const QString STOP_MSG_BTN;
        const QString SAVE_ALL_MSG_BTN;
        const QString START_RECORD_MSG_BTN;
        const QString STOP_RECORD_MSG_BTN;
        const QString CLEAR_WND_MSG_BTN;
        const QString ADD_ANSWER_OBJ_BTN;
        const QString HIDE_ANSWER_OBJ_BTN;
        const QString SHOW_ALL_ANSWER_OBJ_BTN;
        const QString DEL_ALL_ANSWER_OBJ_BTN;
        const QString ACT_FILE_CONFIG_READ;
        const QString ACT_FILE_CONFIG_SAVE;
        const QString ACT_SETTINGS;
        const QString MODE_BOX;

        explicit ActionName(MainWindow* parent)
            : START_MSG_BTN(parent->ui_->start_msg_btn->objectName())
            , STOP_MSG_BTN(parent->ui_->stop_msg_btn->objectName())
            , SAVE_ALL_MSG_BTN(parent->ui_->save_all_msg_btn->objectName())
            , START_RECORD_MSG_BTN(parent->ui_->start_record_msg_btn->objectName())
            , STOP_RECORD_MSG_BTN(parent->ui_->stop_record_msg_btn->objectName())
            , CLEAR_WND_MSG_BTN(parent->ui_->clear_wnd_msg_btn->objectName())
            , ADD_ANSWER_OBJ_BTN(parent->ui_->add_answer_obj_btn->objectName())
            , HIDE_ANSWER_OBJ_BTN(parent->ui_->hide_answer_obj_btn->objectName())
            , SHOW_ALL_ANSWER_OBJ_BTN(parent->ui_->show_all_answer_obj_btn->objectName())
            , DEL_ALL_ANSWER_OBJ_BTN(parent->ui_->del_all_answer_obj_btn->objectName())
            , ACT_FILE_CONFIG_READ(parent->actFileConfigRead_->objectName())
            , ACT_FILE_CONFIG_SAVE(parent->actFileConfigSave_->objectName())
            , ACT_SETTINGS(parent->actSettings_->objectName())
            , MODE_BOX(parent->ui_->mode_cbox->objectName())
        {}
    };

private slots:
    void ActionClicked();

private:
    void StatusBarInit();
    void ToolBarInit();
    void SetupConnections();
    void CreateStatusBarSeparator();

private:
    Ui::MainWindow* ui_;
    QString windowsTitle_;
    CallBackAction action_cb_; // Один callback на все действия.
    QLabel* cnt_all_recive_msg_lbl_;
    QLabel* cnt_can_recive_msg_lbl_;
    QLabel* connect_status_lbl_;
    QLabel* record_text_lbl_;
    QLabel* record_piсture_lbl_;
    QAction* actFileConfigRead_;
    QAction* actFileConfigSave_;
    QAction* actSettings_;
    QLabel* can_info_lbl_;
};
