#pragma once

#include <QWidget>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>

class ColorSquare : public QWidget
{
    Q_OBJECT

public:
    explicit ColorSquare(QWidget* parent = nullptr)
        : QWidget(parent)
        , color_(Qt::white)
    {}

    QColor color() const {
        return color_;
    }

public slots:
    void setColor(const QColor& color) {
        if (color_ != color) {
            color_ = color;
            update(); // Запрашиваем перерисовку.
        }
    }

signals:
    void doubleClicked();

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);

        QColor displayColor;
        if (isEnabled()) {
            displayColor = color_;
        } else {
            // Берём цвет из палитры для отключённого состояния
            displayColor = palette().color(QPalette::Disabled, QPalette::Window);
        }

        painter.fillRect(rect(), displayColor);
        painter.setPen(isEnabled() ? Qt::black : Qt::gray);
        painter.drawRect(rect().adjusted(0, 0, -1, -1));
    }

    // Обработка двойного клика
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        // Можно добавить проверку кнопки, если нужно только на левой кнопке.
        if (event->button() == Qt::LeftButton) {
            emit doubleClicked();
        }
    }

private:
    QColor color_;
};

