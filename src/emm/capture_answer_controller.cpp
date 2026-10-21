#include <QPushButton>
#include <QRandomGenerator>
#include "capture_answer_controller.h"
#include "ui_capture_answer_view.h"
#include "utils.h"

namespace emm {

using namespace cfg;

CaptureAnswerController::CaptureAnswerController(CaptureAnswerView* view,
                                                 CaptureAnswerModel* model,
                                                 QObject* parent)
    : QObject(parent)
    , view_(view)
    , model_(model)
{
    if (!view_) {
        throw std::invalid_argument("CaptureAnswerView cannot be null");
    }

    if (!model_) {
        throw std::invalid_argument("CaptureAnswerModel cannot be null");
    }

    capture_data_controller_ = new HexEditController(view_->GetUi()->capture_data_le, this);
    answer_data_controller_ = new HexEditController(view_->GetUi()->answer_data_le, this);

    // Свернутая форма.
    SetExpandMode(false);

    SetupConnections();
}

// Чекбокс "выбрано"
void CaptureAnswerController::SetSelectCheck(bool checked) {
    view_->GetUi()->select_check->setChecked(checked);
    model_->SetSelectCheck(checked);
}

bool CaptureAnswerController::GetSelectCheck() const {
    return view_->GetUi()->select_check->isChecked();
}

void CaptureAnswerController::selectCheckToggled(bool checked) {
    model_->SetSelectCheck(checked);
}

// Коментарий.
void CaptureAnswerController::SetTextComment(const QString& str) {
    view_->GetUi()->comment_le->setText(str);
}

const QString CaptureAnswerController::GetTextComment() const {
    return view_->GetUi()->comment_le->text();
}

// Идентификатор захвата.
void CaptureAnswerController::SetTextCaptureId(const QString& str) {
    SetTextAndUpdateModel<quint32>(
        str,
        view_->GetUi()->capture_id_le,
        &CaptureAnswerModel::SetCaptureId,
        16
        );
}

const QString CaptureAnswerController::GetTextCaptureId() const {
    return view_->GetUi()->capture_id_le->text();
}

void CaptureAnswerController::captureIdTextChanged(const QString& str) {
    SetTextCaptureId(str);
    emit captureParametersChanged(this);
}

// Данные захвата.
void CaptureAnswerController::SetTextCaptureData(const QString& str) {
    auto capture_data = utils::QStringToQByteArray(str);
    model_->SetCaptureData(capture_data);
    view_->GetUi()->capture_data_le->setText(str);
    capture_data_controller_->SetText(str);
    view_->GetUi()->capture_byte_count_lbl->setText(QString::number(capture_data.size()));
}

const QString CaptureAnswerController::GetTextCaptureData() const {
    return view_->GetUi()->capture_data_le->text();
}

void CaptureAnswerController::captureDataTextChanged(const QString& str) {
    SetTextCaptureData(str);
    emit captureParametersChanged(this);
}

void CaptureAnswerController::captureDataPosChanged(int position) {
    view_->GetUi()->capture_byte_pos_lbl->setText(QString::number(position));
}

// Позиция захвата.
void CaptureAnswerController::SetTextCapturePosition(const QString& str) {
    SetTextAndUpdateModel<int>(
        str,
        view_->GetUi()->capture_position_le,
        &CaptureAnswerModel::SetCapturePosition,
        10
        );
}

const QString CaptureAnswerController::GetTextCapturePosition() const {
    return view_->GetUi()->capture_position_le->text();
}

void CaptureAnswerController::capturePositionTextChanged(const QString& str) {
    SetTextCapturePosition(str);
    emit captureParametersChanged(this);
}

// Количество байт захвата.
void CaptureAnswerController::SetTextCaptureBytesNum(const QString& str) {
    SetTextAndUpdateModel<int>(
        str,
        view_->GetUi()->capture_bytes_num_le,
        &CaptureAnswerModel::SetCaptureBytesNum,
        10
        );
}

const QString CaptureAnswerController::GetTextCaptureBytesNum() const {
    return view_->GetUi()->capture_bytes_num_le->text();
}

void CaptureAnswerController::captureBytesNumTextChanged(const QString& str) {
    SetTextCaptureBytesNum(str);
    emit captureParametersChanged(this);
}

// Идентификатор ответа.
void CaptureAnswerController::SetTextAnswerId(const QString& str) {
    SetTextAndUpdateModel<quint32>(
        str,
        view_->GetUi()->answer_id_le,
        &CaptureAnswerModel::SetAnswerId,
        16
        );
}

const QString CaptureAnswerController::GetTextAnswerId() const {
    return view_->GetUi()->answer_id_le->text();
}

void CaptureAnswerController::answerIdTextChanged(const QString& str) {
    SetTextAnswerId(str);
}

// Данные ответа.
void CaptureAnswerController::SetTextAnswerData(const QString& str) {
    auto answer_data = utils::QStringToQByteArray(str);
    model_->SetAnswerData(answer_data);
    auto pos = view_->GetUi()->answer_data_le->cursorPosition();
    view_->GetUi()->answer_data_le->setText(str);
    view_->GetUi()->answer_data_le->setCursorPosition(pos);
    answer_data_controller_->SetText(str);
    view_->GetUi()->answer_byte_count_lbl->setText(QString::number(answer_data.size()));
    ByteFromIndexToBits();
}

const QString CaptureAnswerController::GetTextAnswerData() const {
    return view_->GetUi()->answer_data_le->text();
}

void CaptureAnswerController::answerDataTextChanged(const QString& str) {
    SetTextAnswerData(str);
}

void CaptureAnswerController::answerDataPosChanged(int position) {
    view_->GetUi()->answer_byte_pos_lbl->setText(QString::number(position));
}

// Позиция начала случайных байт в ответе.
void CaptureAnswerController::SetTextAnswerRandomPosition(const QString& str) {
    SetTextAndUpdateModel<int>(
        str,
        view_->GetUi()->answer_random_position_le,
        &CaptureAnswerModel::SetAnswerRandomPosition,
        10
        );
    ByteFromIndexToBits();
}

const QString CaptureAnswerController::GetTextAnswerRandomPosition() const {
    return view_->GetUi()->answer_random_position_le->text();
}

void CaptureAnswerController::answerRandomPositionTextChanged(const QString& str) {
    SetTextAnswerRandomPosition(str);
}

// Количество случайных байт в ответе
void CaptureAnswerController::SetTextAnswerRandomBytesNum(const QString& str) {
    SetTextAndUpdateModel<int>(
        str,
        view_->GetUi()->answer_random_bytes_num_le,
        &CaptureAnswerModel::SetAnswerRandomBytesNum,
        10
        );
}

const QString CaptureAnswerController::GetTextAnswerRandomBytesNum() const {
    return view_->GetUi()->answer_random_bytes_num_le->text();
}

void CaptureAnswerController::answerRandomBytesNumTextChanged(const QString& str) {
    SetTextAnswerRandomBytesNum(str);
}

// Чекбокс "случайно" для байтов ответа.
void CaptureAnswerController::SetAnswerRandomCheck(bool checked) {
    view_->GetUi()->answer_random_check->setChecked(checked);
    model_->SetAnswerRandomCheck(checked);
}

bool CaptureAnswerController::GetAnswerRandomCheck() const {
    return view_->GetUi()->answer_random_check->isChecked();
}

void CaptureAnswerController::answerRandomCheckToggled(bool checked) {
    model_->SetAnswerRandomCheck(checked);
}

// Битовое поле.
void CaptureAnswerController::answerBitsChanged(uint8_t byte) {
    auto index = model_->GetAnswerRandomPosition();

    if (!index.has_value() || index >= model_->GetAnswerData().size()) {
        return; // Ничего не делаем.
    }

    model_->GetAnswerData()[index.value()] = byte;
    auto str = utils::QByteArrayToQString(model_->GetAnswerData());
    SetTextAnswerData(str);
}

void CaptureAnswerController::ByteFromIndexToBits() {
    auto index = model_->GetAnswerRandomPosition();

    if (!index.has_value()  || index >= model_->GetAnswerData().size()) {
        return; // Ничего не делаем.
    }

    // Отобразим биты.
    view_->GetUi()->bits->SetByte(model_->GetAnswerData()[index.value()]);
}

/*
// Сравнивает принятый из CAN пакет.
bool AnswerCapture::MsgCompare(uint32_t id, uint8_t* data) {
    if (ui_->answer_data_le->Bytes().size() == 0) {
        return false; // Данных для посылки ответа нет.
    }

    if (ui_->capture_data_le->Bytes().size() == 0) {
        return false; // Данных для сравнения нет.
    }

    if (ui_->capture_id_le->GetNumber() != id) {
        return false; // Адрес(идентификатор) не совпал.
    }

    // Проверяем совпадение данных.
    int capture_size = ui_->capture_bytes_num_le->GetNumber(); // По какой длине сравнивать.
    if (capture_size == 0) {
        // Внимание!!! Это временное решение. Нужно придумать как показывать предупреждение в этом и похожем случае.
        // Либо в общей статусной строке с указанием конкретного виджета, либо приделать к виджету метку.
        QTimer::singleShot(0, this, []() {
            QMessageBox::critical(QApplication::activeWindow(), "Ошибка", "Количество данных для сравнения 0!");
        });
        return false;
    }

    int ind = ui_->capture_position_le->GetNumber(); // С какого индекса сравнивать.
    bool comp = true;
    for (int i = 0, j = ind; i < capture_size; ++i, ++j) {
        if (j >= ui_->capture_data_le->Bytes().size()) { // Если превысим размер данных.
            break; // Выход.
        }
        if (data[j] != static_cast<uint8_t>(ui_->capture_data_le->Bytes()[i])) { // Сравнение данных.
            comp = false;
            break;
        }
    }
    return comp;
}

// Наполняет случайными данными ответ, если надо.
void AnswerCapture::RandomizeAnswerData() {
    // Если надо, заполним случайными данными необходимое количество байт.
    if (ui_->answer_random_check->isChecked()) {
        int random_size = ui_->answer_random_bytes_num_le->GetNumber(); // По какой длине рандомизировать.
        int ind = ui_->answer_random_position_le->GetNumber(); // С какого индекса рандомизировать.
        for (int i = 0, j = ind; i < random_size; ++i, ++j) {
            if (j >= ui_->answer_data_le->Bytes().size()) { // Если превысим размер данных.
                break; // Выход.
            }
            ui_->answer_data_le->Bytes()[j] = QRandomGenerator::global()->generate();
//            ui_->answer_data_le->Bytes()[j] += 1; // Сделать еще галочку (или лучше радио-кнопку) "Инкремент"
        }
        ui_->answer_data_le->BytesToText();
    }
}

// Заполняет поле данных ответом с кодами ошибок.
void AnswerCapture::setErrBtnClicked() {
    code_errors_list_.clear();

    // Начальное значение кода ошибки.
    uint16_t code_begin = ui_->begin_value_err_le->GetNumber();

    // Количество ошибок.
    const uint32_t code_cnt = ui_->size_err_le->GetNumber();

    uint16_t status = 0x0001;
    for (uint32_t i = 0; i < code_cnt; ++i) {
        CodeErrors code_errors = {
            code_begin,
            status
        };

        code_errors_list_.append(code_errors);
        ++code_begin;
    }

    ui_->answer_data_le->Bytes().clear();
    ui_->answer_data_le->Bytes().append(0x59);
    ui_->answer_data_le->Bytes().append(0x02);
    ui_->answer_data_le->Bytes().append(code_cnt);
    for (auto it = code_errors_list_.constBegin(); it != code_errors_list_.constEnd(); ++it) {
        const auto& err = *it;
        ui_->answer_data_le->Bytes().append(static_cast<char>(err.code >> 0));
        ui_->answer_data_le->Bytes().append(static_cast<char>(err.code >> 8));
        ui_->answer_data_le->Bytes().append(static_cast<char>(err.status >> 8));
        ui_->answer_data_le->Bytes().append(static_cast<char>(err.status >> 0));
    }

    ui_->answer_data_le->BytesToText();
}

void AnswerCapture::IncrementCodeErrors() {
    if (ui_->autoinc_err_check->isChecked()) {
        if (code_errors_list_.empty()) {
            return;
        }

        ui_->answer_data_le->Bytes().clear();
        ui_->answer_data_le->Bytes().append(0x59);
        ui_->answer_data_le->Bytes().append(0x02);
        ui_->answer_data_le->Bytes().append(code_errors_list_.size());
        for (auto& err : code_errors_list_) {
            err.code += code_errors_list_.size();
            ui_->answer_data_le->Bytes().append(static_cast<char>(err.code >> 0));
            ui_->answer_data_le->Bytes().append(static_cast<char>(err.code >> 8));
            err.status |= 0xE000;
            ui_->answer_data_le->Bytes().append(static_cast<char>(err.status >> 8));
            ui_->answer_data_le->Bytes().append(static_cast<char>(err.status >> 0));
        }
        ui_->answer_data_le->BytesToText();
        ui_->autoinc_err_check->setChecked(false);
    }
}
*/

void CaptureAnswerController::SetCaptureAnswerData(const CaptureAnswerData& data)
{
    SetSelectCheck(data.select);

    SetTextComment(data.comment);

    SetTextCaptureId(data.capture_id);
    SetTextCaptureData(data.capture_data);
    SetTextCapturePosition(data.capture_pos);
    SetTextCaptureBytesNum(data.capture_bytes_num);

    SetTextAnswerId(data.answer_id);
    SetTextAnswerData(data.answer_data);
    SetTextAnswerRandomPosition(data.answer_random_pos);
    SetTextAnswerRandomBytesNum(data.answer_random_bytes_num);

    SetAnswerRandomCheck(data.answer_random_check);
}

CaptureAnswerData CaptureAnswerController::GetCaptureAnswerData() const {
    return {
        GetSelectCheck(),

        GetTextComment(),

        GetTextCaptureId(),
        GetTextCaptureData(),
        GetTextCapturePosition(),
        GetTextCaptureBytesNum(),

        GetTextAnswerId(),
        GetTextAnswerData(),
        GetTextAnswerRandomPosition(),
        GetTextAnswerRandomBytesNum(),

        GetAnswerRandomCheck(),
    };
}

void CaptureAnswerController::SetupConnections() {
    connect(view_->GetUi()->select_check,
            &QCheckBox::toggled,
            this,
            &CaptureAnswerController::selectCheckToggled);

    connect(view_->GetUi()->expand_btn,
            &QPushButton::clicked,
            this,
            &CaptureAnswerController::onExpandBtnClicked);

    connect(view_->GetUi()->delete_btn,
            &QPushButton::clicked,
            this,
            &CaptureAnswerController::onDeleteBtnClicked);

    connect(view_->GetUi()->capture_id_le,
            &QLineEdit::textChanged,
            this,
            &CaptureAnswerController::captureIdTextChanged);

    connect(capture_data_controller_,
            &HexEditController::textChanged,
            this,
            &CaptureAnswerController::captureDataTextChanged);

    connect(capture_data_controller_,
            &HexEditController::bytePositionChanged,
            this,
            &CaptureAnswerController::captureDataPosChanged);

    connect(view_->GetUi()->capture_position_le,
            &QLineEdit::textChanged,
            this,
            &CaptureAnswerController::capturePositionTextChanged);

    connect(view_->GetUi()->capture_bytes_num_le,
            &QLineEdit::textChanged,
            this,
            &CaptureAnswerController::captureBytesNumTextChanged);

    connect(view_->GetUi()->answer_id_le,
            &QLineEdit::textChanged,
            this,
            &CaptureAnswerController::answerIdTextChanged);

    connect(answer_data_controller_,
            &HexEditController::textChanged,
            this,
            &CaptureAnswerController::answerDataTextChanged);

    connect(answer_data_controller_,
            &HexEditController::bytePositionChanged,
            this,
            &CaptureAnswerController::answerDataPosChanged);

    connect(view_->GetUi()->answer_random_position_le,
            &QLineEdit::textChanged,
            this,
            &CaptureAnswerController::answerRandomPositionTextChanged);

    connect(view_->GetUi()->answer_random_bytes_num_le,
            &QLineEdit::textChanged,
            this,
            &CaptureAnswerController::answerRandomBytesNumTextChanged);

    connect(view_->GetUi()->answer_random_check,
            &QCheckBox::toggled,
            this,
            &CaptureAnswerController::answerRandomCheckToggled);

    connect(view_->GetUi()->bits,
            &BitsWidget::checkBitChanged,
            this,
            &CaptureAnswerController::answerBitsChanged);
}

void CaptureAnswerController::onExpandBtnClicked(bool checked) {
    if (checked) {
        SetExpandMode(true);
    } else {
        SetExpandMode(false);
    }
}

// Увеличивает/уменьшает виджет объекта захвата.
void CaptureAnswerController::SetExpandMode(bool mode) {
    view_->GetUi()->capture_gbox->setVisible(mode);
    view_->GetUi()->answer_gbox->setVisible(mode);
}

// Нажата кнопка delete
void CaptureAnswerController::onDeleteBtnClicked() {
    emit deleteObject(this);
}

// Подготавливает данные для ответа в соответствии с установленными
// переключателями рандомизации (для эммулятора).
const QByteArray* CaptureAnswerController::GetPreparedAnswerData() {
    if (!model_->GetAnswerRandomCheck()) { // Рандомизация не нужна.
        return model_->GetAnswerData().size() ? &model_->GetAnswerData() : nullptr;
    }

    // Рандомизация нужна.
    // Получаем параметры рандомизации.
    auto random_size = model_->GetAnswerRandomBytesNum(); // По какой длине рандомизировать.
    if (!random_size.has_value()) {
        return nullptr;  // Нет размера для рандомизации.
    }

    auto ind = model_->GetAnswerRandomPosition(); // С какого индекса рандомизировать.
    if (!ind.has_value()) {
        return nullptr;  // Нет индекса для рандомизации.
    }

    for (int i = 0, j = ind.value(); i < random_size.value(); ++i, ++j) {
        if (j >= model_->GetAnswerData().size()) { // Если превысим размер данных.
            break; // Выход.
        }
        model_->GetAnswerData()[j] = QRandomGenerator::global()->generate();
    }

    // Помещаем данные на Ui.
    auto str = utils::QByteArrayToQString(model_->GetAnswerData());
    SetTextAnswerData(str);

    return &model_->GetAnswerData();
}

} // namespace emm
