#pragma once

#include <QDialog>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "ui_settings_view.h"

namespace Ui {
class SettingsWindow;
}

namespace cfg {

class SettingsWindow : public QDialog
{
    Q_OBJECT

    using CallBackAction = std::function<void(const QObject* qobject_action)>;

public:
    static constexpr int CAN_ID_MAX_LENGTH = 8;

    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    void SetActionCallback(std::function<void(const QObject* qobject_action)> cb);

    struct ActionName {
        const QString SAVE_BTN;
        const QString CANCEL_BTN;
        const QString LOG_DIR_BTN;
        const QString SOFT_FILTER_ADD_ID_BTN;
        const QString SOFT_FILTER_UPDATE_ID_BTN;
        const QString SOFT_FILTER_DELETE_ID_ALL_BTN;
        const QString SOFT_FILTER_DELETE_ID_SELECT_BTN;
        const QString HARD_FILTER_ADD_ID_BTN;
        const QString HARD_FILTER_UPDATE_ID_BTN;
        const QString HARD_FILTER_DELETE_ID_ALL_BTN;
        const QString HARD_FILTER_DELETE_ID_SELECT_BTN;
        const QString COLORING_FILTER_ADD_ID_BTN;
        const QString COLORING_FILTER_UPDATE_ID_BTN;
        const QString COLORING_FILTER_DELETE_ID_ALL_BTN;
        const QString COLORING_FILTER_DELETE_ID_SELECT_BTN;
        const QString COLORING_FILTER_MODE_CHECK;

        explicit ActionName(SettingsWindow* parent)
            : SAVE_BTN(parent->ui_->save_btn->objectName())
            , CANCEL_BTN(parent->ui_->cancel_btn->objectName())
            , LOG_DIR_BTN(parent->ui_->log_dir_btn->objectName())
            , SOFT_FILTER_ADD_ID_BTN(parent->ui_->soft_filter_add_id_btn->objectName())
            , SOFT_FILTER_UPDATE_ID_BTN(parent->ui_->soft_filter_update_id_btn->objectName())
            , SOFT_FILTER_DELETE_ID_ALL_BTN(parent->ui_->soft_filter_delete_id_all_btn->objectName())
            , SOFT_FILTER_DELETE_ID_SELECT_BTN(parent->ui_->soft_filter_delete_id_select_btn->objectName())
            , HARD_FILTER_ADD_ID_BTN(parent->ui_->hard_filter_add_id_btn->objectName())
            , HARD_FILTER_UPDATE_ID_BTN(parent->ui_->hard_filter_update_id_btn->objectName())
            , HARD_FILTER_DELETE_ID_ALL_BTN(parent->ui_->hard_filter_delete_id_all_btn->objectName())
            , HARD_FILTER_DELETE_ID_SELECT_BTN(parent->ui_->hard_filter_delete_id_select_btn->objectName())
            , COLORING_FILTER_ADD_ID_BTN(parent->ui_->coloring_filter_add_id_btn->objectName())
            , COLORING_FILTER_UPDATE_ID_BTN(parent->ui_->coloring_filter_update_id_btn->objectName())
            , COLORING_FILTER_DELETE_ID_ALL_BTN(parent->ui_->coloring_filter_delete_id_all_btn->objectName())
            , COLORING_FILTER_DELETE_ID_SELECT_BTN(parent->ui_->coloring_filter_delete_id_select_btn->objectName())
            , COLORING_FILTER_MODE_CHECK(parent->ui_->coloring_mode_check->objectName())
        {}
    };

    Ui::SettingsWindow* GetUi() {
        return ui_;
    }

private:
    void SetupConnections();
    void ActionClicked();
    QRegularExpressionValidator* CreateHexValidator();

private:
    Ui::SettingsWindow* ui_;
    CallBackAction action_cb_ = nullptr; // Один callback на все действия.
    QRegularExpressionValidator* hex_validator_;
};

} // namespace cfg
