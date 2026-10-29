#pragma once

#include <QObject>
#include "capture_answer_view.h"
#include "capture_answer_model.h"
#include "config.h"
#include "hex_edit_controller.h"
#include "string_converter.h"

namespace emm {

class CaptureAnswerController : public QObject
{
    Q_OBJECT

public:
    explicit CaptureAnswerController(CaptureAnswerView* view,
                                     CaptureAnswerModel* model,
                                     QObject* parent = nullptr);
    ~CaptureAnswerController() = default;

    // Запрещаем копирование.
    CaptureAnswerController(const CaptureAnswerController&) = delete;
    CaptureAnswerController& operator=(const CaptureAnswerController&) = delete;
    // Запрещаем перемещение.
    CaptureAnswerController(CaptureAnswerController&& other) = delete;
    CaptureAnswerController& operator=(CaptureAnswerController&& other) = delete;

    void SetSelectCheck(bool checked);
    bool GetSelectCheck() const;
    const QString GetTextComment() const;
    void SetTextComment(const QString& str);
    void SetTextCaptureId(const QString& str);
    const QString GetTextCaptureId() const;
    void SetTextCaptureData(const QString& str);
    const QString GetTextCaptureData() const;
    void SetTextCapturePosition(const QString& str);
    const QString GetTextCapturePosition() const;
    void SetTextCaptureBytesNum(const QString& str);
    const QString GetTextCaptureBytesNum() const;
    void SetTextAnswerId(const QString& str);
    const QString GetTextAnswerId() const;
    void SetTextAnswerData(const QString& str);
    const QString GetTextAnswerData() const;
    void SetTextAnswerRandomPosition(const QString& str);
    const QString GetTextAnswerRandomPosition() const;
    void SetTextAnswerRandomBytesNum(const QString& str);
    const QString GetTextAnswerRandomBytesNum() const;
    void SetAnswerRandomCheck(bool checked);
    bool GetAnswerRandomCheck() const;
    void SetExpandMode(bool mode);

    void SetCaptureAnswerData(const cfg::CaptureAnswerData& data);
    cfg::CaptureAnswerData GetCaptureAnswerData() const;

    const QByteArray* GetPreparedAnswerData();

    CaptureAnswerView* GetView() const {
        return view_;
    }

    CaptureAnswerModel* GetModel() const {
        return model_;
    }

signals:
    void deleteObject(emm::CaptureAnswerController* ptr);
    void captureParametersChanged(emm::CaptureAnswerController* ptr);

private slots:
    void selectCheckToggled(bool checked);
    void onExpandBtnClicked(bool checked);
    void onDeleteBtnClicked();
    void captureIdTextChanged(const QString& str);
    void captureDataTextChanged(const QString& str);
    void captureDataPosChanged(int position);
    void capturePositionTextChanged(const QString& str);
    void captureBytesNumTextChanged(const QString& str);
    void answerIdTextChanged(const QString& str);
    void answerDataTextChanged(const QString& str);
    void answerDataPosChanged(int position);
    void answerRandomPositionTextChanged(const QString& str);
    void answerRandomBytesNumTextChanged(const QString& str);
    void answerRandomCheckToggled(bool checked);
    void answerBitsChanged(uint8_t byte);

private:
    void SetupConnections();
    void ByteFromIndexToBits();

    template<typename ValueType, typename SetterFunc, typename WidgetPtr>
    void SetTextAndUpdateModel(
        const QString& str,
        WidgetPtr widget,
        SetterFunc setter,
        int base = 10
        ) {
        widget->setText(str);
        bool ok;
        ValueType value = string_converter::to<ValueType>(str, &ok, base);

        if (ok) {
            (model_->*setter)(value);
        } else {
            (model_->*setter)(std::nullopt);
        }
    }

private:
    CaptureAnswerView* view_ = nullptr;
    CaptureAnswerModel* model_ = nullptr;
    HexEditController* capture_data_controller_ = nullptr;
    HexEditController* answer_data_controller_ = nullptr;
};

} // namespace emm

