#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include "utils.h"
#include "hex_edit_controller.h"

HexEditController::HexEditController(HexEditView* view, QObject* parent)
    : QObject(parent)
    , view_(view)
{
    style_narrow = new LineEditStyle(1, view_->style());
    style_wide = new LineEditStyle(GetSymbolWidth('F'), view_->style());

    view_->setStyle(style_narrow);
    view_->setContextMenuPolicy(Qt::NoContextMenu);
    view_->setMaxLength(MAX_BYTES_NUM * 3);

    SetupConnections();
}

void HexEditController::SetupConnections() {
    connect(view_, &HexEditView::KeyPress, this, &HexEditController::keyPress);
    connect(view_, &HexEditView::KeyRelease, this, &HexEditController::keyRelease);
    connect(view_, &HexEditView::MouseRelease, this, &HexEditController::mouseRelease);
    connect(view_, &QLineEdit::cursorPositionChanged, this, &HexEditController::cursorPositionChanged);
}

void HexEditController::keyPress(const int key, const QString& text) {
    ReadOnly read_only(view_); // Чтобы избавиться от мигания курсора при перемещении.

    // Ввод числа.
    if ((text >= '0' && text <= '9')
        || (text >= 'A' && text <= 'F')
        || (text >= 'a' && text <= 'f')) {
        SaveBeforeChanging();
        PrintDigit(text);
    }
    // Вкл/выкл режима вставки/замены. Shift не нажата.
    else if (!key_shift_press_ && key == Qt::Key_Insert) {
        InsertModeOnOff();
    }
    // Перемещение курсора влево в режиме вставки без выделения. Shift не нажат.
    else if (key == Qt::Key_Left && view_->cursorPosition() > 0 && insert_mode_ && !key_shift_press_) {
        StepCursorLeft();
        sel_begin_ = view_->cursorPosition();
    }
    // Перемещение курсора вправо в режиме вставки без выделения. Shift не нажат.
    else if (key == Qt::Key_Right && view_->cursorPosition() < text_.size() && insert_mode_ && !key_shift_press_) {
        StepCursorRight();
        sel_begin_ = view_->cursorPosition();
    }
    // Перемещение курсора влево в режиме вставки с выделением. Shift нажат.
    else if (key == Qt::Key_Left && view_->cursorPosition() > 0 && insert_mode_ && key_shift_press_) {
        StepCursorLeft();
        view_->setSelection(sel_begin_, view_->cursorPosition() - sel_begin_);
    }
    // Перемещение курсора вправо в режиме вставки с выделением. Shift нажат.
    else if (key == Qt::Key_Right && view_->cursorPosition() < text_.size() && insert_mode_ && key_shift_press_) {
        StepCursorRight();
        view_->setSelection(sel_begin_, view_->cursorPosition() - sel_begin_);
    }
    // Перемещение курсора влево в режиме замены. Shift не нажат.
    else if (key == Qt::Key_Left && view_->cursorPosition() > 0 && !insert_mode_ && !key_shift_press_) {
        StepCursorLeft();
    }
    // Перемещение курсора вправо в режиме замены. Shift не нажат.
    else if (key == Qt::Key_Right && view_->cursorPosition() + 2 < text_.size() && !insert_mode_ && !key_shift_press_) {
        StepCursorRight();
    }
    // Нажатие Shift.
    else if (key == Qt::Key_Shift) {
        key_shift_press_ = true; // Флаг начала нажатия Shift.
    }
    // Нажатие Esc - отмена выделения.
    else if (key == Qt::Key_Escape) {
        view_->deselect();
        sel_begin_ = view_->cursorPosition();
        ClearUserSelection();
    }
    // Нажатие Ctrl.
    else if (key == Qt::Key_Control) {
        key_ctrl_press_ = true; // Флаг начала нажатия Ctrl.
    }
    // Выделить все. Ctrl нажат. Только в режиме вставки.
    else if (key_ctrl_press_ && key == Qt::Key_A && insert_mode_) {
        view_->setSelection(0, text_.size());
        sel_begin_ = view_->cursorPosition();
    }
    // Выделить от позиции курсора до конца. Shift нажат. Только в режиме вставки.
    else if (key_shift_press_ && key == Qt::Key_End && insert_mode_) {
        view_->setSelection(view_->cursorPosition(), text_.size());
    }
    // Выделить от позиции курсора до начала. Shift нажат. Только в режиме вставки.
    else if (key_shift_press_ && key == Qt::Key_Home && insert_mode_) {
        view_->setSelection(0, view_->cursorPosition());
    }
    // Удалить выделенное. Только в режиме вставки.
    else if (!key_shift_press_ && key == Qt::Key_Delete && insert_mode_) {
        SaveBeforeChanging();
        DeleteSelection();
    }
    // Перемещение в начало.
    else if (key == Qt::Key_Home) {
        view_->setCursorPosition(0);
        sel_begin_ = view_->cursorPosition();
    }
    // Перемещение в конец. Режим вставки.
    else if (key == Qt::Key_End && insert_mode_) {
        view_->setCursorPosition(text_.size());
        sel_begin_ = view_->cursorPosition();
    }
    // Перемещение в конец. Режим замены.
    else if (key == Qt::Key_End && !insert_mode_) {
        view_->setCursorPosition(text_.size() - 2);
    }
    // Удаление байта перед курсором. Режим вставки.
    else if (key == Qt::Key_Backspace && insert_mode_) {
        SaveBeforeChanging();
        PressBackspace();
    }
    // Выделение байта (метка на байт).
    else if (key == Qt::Key_Return) {
        ByteSelection();
    }
    // Восстановление предыдущего состояния строки.
    else if (key_ctrl_press_ && key == Qt::Key_Z) {
        RestoreСhanges();
    }
    // Вырезание выделенной части. Сохраняем в буфер обмена.
    else if (key_ctrl_press_ && key == Qt::Key_X) {
        SaveBeforeChanging();
        CopySelection();
        DeleteSelection();
    }
    else if (key_shift_press_ && key == Qt::Key_Delete) {
        SaveBeforeChanging();
        CopySelection();
        DeleteSelection();
    }
    // Сохраняем в буфер обмена выделенную часть.
    else if (key_ctrl_press_ && key == Qt::Key_C) {
        CopySelection();
    }
    // Вставляем строку из буфера обмена.
    else if (key_ctrl_press_ && key == Qt::Key_V) {
        SaveBeforeChanging();
        Paste();
        // Необъяснимо пока. Почему-то подвисает нажатие Shift и Ctrl внутри Paste().
        // Сбросим принудительно.
        key_ctrl_press_ = false;
    }
    else if (key_shift_press_ && key == Qt::Key_Insert) {
        SaveBeforeChanging();
        Paste();
        // Необъяснимо пока. Почему-то подвисает нажатие Shift и Ctrl внутри Paste().
        // Сбросим принудительно.
        key_shift_press_ = false;
    }
}

void HexEditController::keyRelease(const int key) {
    if (key == Qt::Key_Shift) { // Отпускание Shift.
        key_shift_press_ = false;
    } else if (key == Qt::Key_Control) { // Отпускание Ctrl.
        key_ctrl_press_ = false;
    }
}

void HexEditController::mouseRelease() {
    sel_begin_ = view_->cursorPosition();
}

void HexEditController::SaveBeforeChanging() {
    undo_.str = text_;
    undo_.cursor_pos = view_->cursorPosition();
    undo_.is_repit = is_repit_;
    if (view_->hasSelectedText()) {
        undo_.sel_begin = view_->selectionStart();
        undo_.sel_length = view_->selectionLength();
    } else {
        undo_.sel_begin = 0;
        undo_.sel_length = 0;
    }
}

void HexEditController::InsertModeOnOff() {
    is_repit_ = false;
    if (insert_mode_ && text_.size()) { // Только если есть текст.
        // Режим замены символа.
        insert_mode_ = false;

        // В режиме замены сдвинем курсор в позицию последнего символа.
        if (view_->cursorPosition() == text_.size()) {
            view_->setCursorPosition(view_->cursorPosition() - 2);
        } else if (view_->cursorPosition() + 1 == text_.size()) {
            view_->setCursorPosition(view_->cursorPosition() - 1);
        }

        // Курсор в виде прямоугольника.
        view_->setStyle(style_wide);


        // Отменим выделение.
        view_->deselect();
    } else {
        // Режим вставки символа.
        insert_mode_ = true;

        // Если курсор находится на 1-й цифре.
        if (CheckDigit1(view_->cursorPosition())) {
            view_->setCursorPosition(view_->cursorPosition() - 1); // Сдвинем его влево.
        }

        // Курсор в виде палочки.
        view_->setStyle(style_narrow);
    }
    sel_begin_ = view_->cursorPosition();
}

void HexEditController::PrintDigit(QString text) {
    // Режим вставки и курсор в позиции 0-й цифры.
    if (insert_mode_ && CheckDigit0(view_->cursorPosition())) {
        if (!is_repit_) { // Печатаем первую цифру.
            text_.insert(view_->cursorPosition(), '0'); // Добавляем 0 впереди.
            text_.insert(view_->cursorPosition() + 1, text);
            text_.insert(view_->cursorPosition() + 2, ' ');
            auto cursor_position = view_->cursorPosition();
            TextToView();
            view_->setCursorPosition(cursor_position);
            is_repit_ = true;
        } else { // Печатаем вторую цифру этого байта.
            text_.remove(view_->cursorPosition(), 1);
            text_.insert(view_->cursorPosition() + 1, text);
            auto cursor_position = view_->cursorPosition();
            TextToView();
            view_->setCursorPosition(cursor_position + 3);
            is_repit_ = false;
        }
    }
    // Если режим замены и в позиции курсора не пробел.
    else if (!insert_mode_ && !CheckSpace(view_->cursorPosition())) {
        auto cursor_position = view_->cursorPosition();
        text_.remove(cursor_position, 1);
        text_.insert(cursor_position, text);
        TextToView();
        view_->setCursorPosition(cursor_position + 1);
    }
    sel_begin_ = view_->cursorPosition();
}

void HexEditController::DeleteSelection() {
    if (!view_->hasSelectedText()) {
        return;
    }

    auto sel_start = view_->selectionStart();
    text_.remove(view_->selectionStart(), view_->selectionLength());
    TextToView();
    view_->setCursorPosition(sel_start);
}

void HexEditController::CopySelection() {
    if (!view_->hasSelectedText()) {
        return;
    }

    auto clipboard = QApplication::clipboard();
    clipboard->setText(text_.mid(view_->selectionStart(), view_->selectionLength()));
}

void HexEditController::Paste() {
    auto clipboard = QApplication::clipboard();

    QString qs = clipboard->text();
    if (!utils::CheckTextBytesFormat(qs)) {
        QMessageBox::critical(QApplication::activeWindow(), "Ошибка", "Неправильный Hex-формат строки");
        return;
    }

    qs.append(' '); // Добавим пробел в конце, чтобы поддержать наш формат.
    DeleteSelection();
    auto cursor_position = view_->cursorPosition();
    text_.insert(view_->cursorPosition(), qs);
    TextToView();
    view_->setCursorPosition(cursor_position + qs.size());
    sel_begin_ = view_->cursorPosition();
}

void HexEditController::PressBackspace() {
    text_.remove(view_->cursorPosition() - 3, 3);
    auto cursor_position = view_->cursorPosition();
    TextToView();
    view_->setCursorPosition(cursor_position - 3);
    sel_begin_ = view_->cursorPosition();
}

void HexEditController::ByteSelection() {
    if (!insert_mode_ && CheckDigit1(view_->cursorPosition())) {
        SetUserSelection(view_->cursorPosition() - 1, 2);
    } else {
        SetUserSelection(view_->cursorPosition(), 2);
    }
}

// Проверяет, принадлежит ли индекс 0-й цифре.
bool HexEditController::CheckDigit0(const int index) {
    if (index % 3) {
        return false;
    }
    return true;
}

// Проверяет, принадлежит ли индекс 1-й цифре.
bool HexEditController::CheckDigit1(const int index) {
    if ((index - 1)  % 3) {
        return false;
    }
    return true;
}

// Проверяет, принадлежит ли индекс пробелу.
bool HexEditController::CheckSpace(const int index) {
    if ((index - 2)  % 3) {
        return false;
    }
    return true;
}

void HexEditController::StepCursorLeft() {
    auto cursor_position = view_->cursorPosition() - 1;

    if (insert_mode_) {
        while (!CheckDigit0(cursor_position)) {
            --cursor_position;
        }
    } else {
        while (CheckSpace(cursor_position)) {
            --cursor_position;
        }
        view_->setStyle(style_wide);
    }

    view_->setCursorPosition(cursor_position);
}

void HexEditController::StepCursorRight() {
    auto cursor_position = view_->cursorPosition() + 1;

    if (insert_mode_) {
        while (!CheckDigit0(cursor_position)) {
            ++cursor_position;
        }
    } else {
        while (CheckSpace(cursor_position)) {
            ++cursor_position;
        }
        view_->setCursorPosition(cursor_position);
        view_->setStyle(style_wide);
    }

    view_->setCursorPosition(cursor_position);
}

void HexEditController::SetUserSelection(const int start, const int lenght) {
    QList<QTextLayout::FormatRange> formats;
    QTextCharFormat f;

    f.setBackground(QColor(195, 195, 195)); // Цвет выделения, вынести в глобальные настройки!
    f.setForeground(Qt::black);
    QTextLayout::FormatRange fr_tracker;
    fr_tracker.start = start;
    fr_tracker.length = std::abs(lenght);
    fr_tracker.format = f;

    formats.append(fr_tracker);
    utils::SetLineEditTextFormat(view_, formats);
}

void HexEditController::ClearUserSelection() {
    utils::ClearLineEditTextFormat(view_);
}

void HexEditController::RestoreСhanges() {
    if (undo_.str.isEmpty()) {
        return;
    }

    text_ = undo_.str;
    TextToView();
    view_->setCursorPosition(undo_.cursor_pos);
    if (undo_.sel_length) {
        view_->setSelection(undo_.sel_begin, undo_.sel_length);
    }

    undo_.str.clear();
    sel_begin_ = view_->cursorPosition();
    is_repit_ = undo_.is_repit;
}

void HexEditController::TextToView() {
    text_ = text_.toUpper();
    //    view_->setText(text_);

    emit textChanged(text_);
}

void HexEditController::SetText(const QString& text) {
    text_ = text;
}

void HexEditController::cursorPositionChanged(int oldPos, int newPos) {
    Q_UNUSED(oldPos)

    if (insert_mode_) {
        if (CheckDigit1(newPos)) {
            view_->setCursorPosition(newPos - 1);
        } else if (CheckSpace(newPos)) {
            view_->setCursorPosition(newPos + 1);
        }
    } else {
        if (newPos == text_.size()) {
            view_->setCursorPosition(newPos - 2);
        } else if (CheckSpace(newPos)) {
            view_->setCursorPosition(newPos + 1);
        }
    }

    byte_position_ = view_->cursorPosition() / 3;
    emit bytePositionChanged(byte_position_);
}

int HexEditController::GetSymbolWidth(int cursor_position) const {
    QFontMetrics fm(view_->font());

    if (cursor_position < text_.length()) {
        QChar sym = text_.at(cursor_position);
        return fm.horizontalAdvance(sym);
    }

    return 7;
}

