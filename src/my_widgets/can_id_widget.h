#pragma once

#include <QWidget>
#include <QString>
#include <QColor>
#include <QMouseEvent>
#include <QPainter>

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
        , mouse_pressed_(false)
        , selected_(false)
    {
        ui_->setupUi(this);

        connect(ui_->color_wgt, &ColorSquare::doubleClicked, this, [this](){
            emit mouseColorDoubleClick();
        });
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

    bool GetSelected() const {
        return selected_;
    }

    void SetColorVisible(bool visible) {
        ui_->color_wgt->setVisible(visible);
    }

    void SetTextHeaderType(const QString& text) {
        ui_->header_type_lbl->setText(text);
    }

    const QString GetTextHeaderType() const {
        return ui_->header_type_lbl->text();
    }

signals:
    void mouseClick();
    void mouseColorDoubleClick();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::CanIdWidget *ui_;
    bool mouse_pressed_;
    bool selected_;
};

