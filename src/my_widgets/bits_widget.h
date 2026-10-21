#pragma once

#include <QWidget>
#include <QList>

class QCheckBox;

class BitsWidget : public QWidget
{
    Q_OBJECT

public:
    BitsWidget(QWidget* parent = nullptr);
    void SetByte(uint8_t data);
    uint8_t GetByte() const;

public slots:
    void checkBitClicked();

signals:
    void checkBitChanged(uint8_t byte);

private:
    QList<QCheckBox*> check_bit_list_;
};
