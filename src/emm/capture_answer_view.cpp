#include <QRandomGenerator>
#include <QMessageBox>
#include <QDebug>

#include "capture_answer_view.h"
#include "ui_capture_answer_view.h"

namespace emm {

CaptureAnswerView::CaptureAnswerView(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::CaptureAnswerView)
{
    ui_->setupUi(this);

    // Создаём валидаторы один раз
    hex_validator_ = CreateHexValidator();
    int_validator_ = CreateIntValidator();

    // hex-поля
    SetupLineEdit(ui_->capture_id_le, validatorType::HEX, 8);
    SetupLineEdit(ui_->answer_id_le, validatorType::HEX, 8);
    SetupLineEdit(ui_->begin_value_err_le, validatorType::HEX, 4);
    // int-поля
    SetupLineEdit(ui_->capture_position_le, validatorType::INT, 3);
    SetupLineEdit(ui_->capture_bytes_num_le, validatorType::INT, 3);
    SetupLineEdit(ui_->answer_random_position_le, validatorType::INT, 3);
    SetupLineEdit(ui_->answer_random_bytes_num_le, validatorType::INT, 3);
    SetupLineEdit(ui_->size_err_le, validatorType::INT, 3);
}

CaptureAnswerView::~CaptureAnswerView() {
    delete ui_;
}

void CaptureAnswerView::SetupLineEdit(QLineEdit *line_edit,
                                      validatorType validator_type,
                                      int max_length)
{
    line_edit->setMaxLength(max_length);

    QChar maxChar;
    if (validator_type == validatorType::HEX) {
        line_edit->setValidator(hex_validator_);
        maxChar = 'F'; // Для hex-полей.
    } else {
        line_edit->setValidator(int_validator_);
        maxChar = '9'; // Для int-полей.
    }

    QString sampleText(max_length, maxChar);
    QFontMetrics fm(line_edit->font());
    int textWidth = fm.horizontalAdvance(sampleText);

    // Добавляем отступ.
    line_edit->setFixedWidth(textWidth + EXTRA_WIDTH);
}

QRegularExpressionValidator* CaptureAnswerView::CreateHexValidator() {
    QRegularExpression hexRegex("^[0-9A-F]*$");
    return new QRegularExpressionValidator(hexRegex, this);
}

QRegularExpressionValidator* CaptureAnswerView::CreateIntValidator() {
    QRegularExpression intRegexp("[0-9]{1,3}");
    return new QRegularExpressionValidator(intRegexp, this);
}

} // namespace emm


