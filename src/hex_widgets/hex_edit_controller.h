#pragma once

#include <QObject>
#include "hex_edit_view.h"

class HexEditController : public QObject
{
    Q_OBJECT

public:
    explicit HexEditController(HexEditView* view, QObject* parent = nullptr);
    ~HexEditController() {
        delete style_narrow;
        delete style_wide;
    };

    HexEditController(HexEditController& other) = delete;
    HexEditController& operator=(HexEditController& other) = delete;
    HexEditController(HexEditController&& other) = delete;
    HexEditController& operator=(HexEditController&& other) = delete;

    void SetText(const QString& text);

signals:
    void bytePositionChanged(int position);
    void textChanged(const QString& str);

private slots:
    void keyPress(const int key, const QString& text);
    void keyRelease(const int key);
    void mouseRelease();
    void cursorPositionChanged(int oldPos, int newPos);

private:
    void SetupConnections();
    bool CheckDigit0(const int index);
    bool CheckDigit1(const int index);
    bool CheckSpace(const int index);
    void SetUserSelection(const int start, const int lenght);
    void ClearUserSelection();
    void StepCursorLeft();
    void StepCursorRight();
    void DeleteSelection();
    void CopySelection();
    void Paste();
    void PrintDigit(QString text);
    void InsertModeOnOff();
    void PressBackspace();
    void ByteSelection();
    void SaveBeforeChanging();
    void RestoreСhanges();
    void TextToView();
    int GetSymbolWidth(int cursor_position) const;

private:
    class LineEditStyle : public QProxyStyle
    {
    public:
        LineEditStyle(int cursor_width, QStyle *style = 0)
            : QProxyStyle(style)
            , cursor_width_(cursor_width) {
        }

        int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const {
            if (metric == QStyle::PM_TextCursorWidth)
                return cursor_width_;

            return QProxyStyle::pixelMetric(metric, option, widget);
        }

    private:
        int cursor_width_;
    };

private:
    struct Undo {
        QString str;
        int cursor_pos;
        int sel_begin;
        int sel_length;
        bool is_repit;
    };

    class ReadOnly {
    public:
        ReadOnly(HexEditView* view) : view_(view) {
            if (!view_) {
                return;
            }
            view_->setReadOnly(true); // Запрет перерисовки.
        }

        ~ReadOnly() {
            if (!view_) {
                return;
            }
            view_->setReadOnly(false); // Разрешение перерисовки.
        }

    private:
        HexEditView* view_ = nullptr;
    };

    static constexpr int MAX_BYTES_NUM = 2048;
    HexEditView* view_ = nullptr;
    QString text_;
    bool insert_mode_ = true; // Режим вставки/замены.
    bool is_repit_ = false; // Флаг повторного ввода цифры для этого же байта.
    bool key_shift_press_ = false; // Клавиша Shift нажата/не нажата.
    bool key_ctrl_press_ = false; // Клавиша Ctrl нажата/не нажата.
    int sel_begin_ = 0; // Индекс начала выделения.
    int sel_length_ = 0; // Длина выделения.
    Undo undo_; // История строки. Приделать контейнер!!!
    int byte_position_ = 0;
    int size_limit_ = 0;
    int old_byte_size_ = 0;
    LineEditStyle* style_narrow;
    LineEditStyle* style_wide;
};
