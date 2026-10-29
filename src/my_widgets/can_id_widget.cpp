#include "can_id_widget.h"

void CanIdWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Выбираем цвет фона.
    QColor bgColor;
    if (selected_) {
        bgColor = QColor(205, 205, 205); // Выбрано.
    } else {
        bgColor = QColor(240, 240, 240); // Не выбрано.
    }

    painter.fillRect(rect(), bgColor);

    QWidget::paintEvent(event);
}

void CanIdWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mouse_pressed_ = true;
    }
    QWidget::mousePressEvent(event);
}

void CanIdWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mouse_pressed_
        && event->button() == Qt::LeftButton
        && rect().contains(event->pos()))
    {
        selected_ = !selected_;

        emit mouseClick();

        update();
    }

    mouse_pressed_ = false;
    QWidget::mouseReleaseEvent(event);
}
