#include "hex_edit_view.h"

HexEditView::HexEditView(QWidget* parent) : QLineEdit(parent) {
}

void HexEditView::keyPressEvent(QKeyEvent *event) {
    emit KeyPress(event->key(), event->text());
}

void HexEditView::keyReleaseEvent(QKeyEvent *event) {
    emit KeyRelease(event->key());
}

void HexEditView::mouseReleaseEvent(QMouseEvent *event) {
    QLineEdit::mouseReleaseEvent(event);
    emit MouseRelease();
}

