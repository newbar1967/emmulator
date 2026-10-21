#pragma once

#include <QWidget>
#include <QColor>
#include <QPainter>

class ColorSquare : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit ColorSquare(QWidget* parent = nullptr)
        : QWidget(parent)
        , m_color_(Qt::white)
    {}

    QColor color() const {
        return m_color_;
    }

public slots:
    void setColor(const QColor& color) {
        if (m_color_ != color) {
            m_color_ = color;
            update(); // Запрашиваем перерисовку.
            emit colorChanged(color);
        }
    }

signals:
    void colorChanged(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);

        QColor displayColor;
        if (isEnabled()) {
            displayColor = m_color_;
        } else {
            // Берём цвет из палитры для отключённого состояния
            displayColor = palette().color(QPalette::Disabled, QPalette::Window);
        }

        painter.fillRect(rect(), displayColor);
        painter.setPen(isEnabled() ? Qt::black : Qt::gray);
        painter.drawRect(rect().adjusted(0, 0, -1, -1));
    }

private:
    QColor m_color_;
};

