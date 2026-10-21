#include <QFrame>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>

#include "bits_widget.h"

BitsWidget::BitsWidget(QWidget* parent)
    : QWidget(parent)
{
    auto frame = new QFrame;
    frame->setFrameShape(QFrame::Box);
    frame->setFrameShadow(QFrame::Sunken);
    frame->setFixedSize(140, 24);

    auto layout_check = new QHBoxLayout;

    layout_check->setContentsMargins(2, 0, 0, 0);
    layout_check->setSpacing(0);
    for (size_t i = 0; i < 4; ++i) {
        auto check = new QCheckBox;
        check->setFixedSize(16, 16);
        layout_check->addWidget(check);
        check_bit_list_.append(check);
        connect(check, &QCheckBox::clicked, this, &BitsWidget::checkBitClicked);
    }
//    layout_check->addWidget(new QLabel(".")); // Точка.
    auto line = new QFrame;
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Raised);
    layout_check->addWidget(line);
    layout_check->addSpacing(3);
    for (size_t i = 4; i < 8; ++i) {
        auto check = new QCheckBox;
        check->setFixedSize(16, 16);
        layout_check->addWidget(check);
        check_bit_list_.append(check);
        connect(check, &QCheckBox::clicked, this, &BitsWidget::checkBitClicked);
    }
    frame->setLayout(layout_check);

    auto layout_main = new QHBoxLayout;
    layout_main->setContentsMargins(0, 0, 0, 0);
    layout_main->setSpacing(0);
    layout_main->addWidget(frame);

    setLayout(layout_main);
}

// Отображает содержимое байта.
void BitsWidget::SetByte(uint8_t data)
{
    for (auto check : check_bit_list_) {
        if (data & 0x80) {
            check->setChecked(true);
        } else {
            check->setChecked(false);
        }
        data <<= 1;
    }
}

// Состояние check box конвертирует в байт.
uint8_t BitsWidget::GetByte() const
{
    uint8_t data = 0;
    uint8_t cnt = 0x80;
    for (auto check : check_bit_list_) {
        if (check->isChecked()) {
            data |= cnt;
        } else {
            data &= cnt ^ 0xFF;
        }
        cnt >>= 1;
    }

    return data;
}

void BitsWidget::checkBitClicked()
{
    emit checkBitChanged(GetByte());
}
