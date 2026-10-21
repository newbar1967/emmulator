#pragma once

#include <QWidget>
#include <QString>
#include <QColor>

#include "ui_can_id_widget.h"

namespace Ui {
class CanIdWidget;
}

class CanIdWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CanIdWidget(QWidget *parent = nullptr)
        : QWidget(parent)
        , ui_(new Ui::CanIdWidget)
    {
        ui_->setupUi(this);
    }

    ~CanIdWidget() {
        delete ui_;
    }

    Ui::CanIdWidget* GetUi() {
        return ui_;
    }

    const QString GetIdStr() const {
        return ui_->can_id_lbl->text();
    }

    void SetIdStr(const QString& str) {
        ui_->can_id_lbl->setText(str);
    }

    const QColor GetColor() const {
        return ui_->color_wgt->color();
    }

    void SetColor(const QColor& color) {
        ui_->color_wgt->setColor(color);
    }

    bool GetSelect() {
        return ui_->select_check->checkState();
    }

    void SetColorVisible(bool visible) {
        ui_->color_wgt->setVisible(visible);
    }

private:
    Ui::CanIdWidget *ui_;
};
