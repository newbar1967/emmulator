#pragma once

#include <QObject>
#include <QByteArray>
#include <optional>
#include "utils.h"

namespace emm {

class CaptureAnswerModel : public QObject
{
    Q_OBJECT

public:
    CaptureAnswerModel(QObject* parent = nullptr);

    void SetSelectCheck(bool checked) {
        select_check_ = checked;
    }

    bool GetSelectCheck() const {
        return select_check_;
    }

    void SetCaptureId(std::optional<quint32> id) {
        capture_id_ = id;
    }

    std::optional<quint32> GetCaptureId() const {
        return capture_id_;
    }

    void SetCaptureData(const QByteArray& byte_array) {
        capture_data_ = byte_array;
    }

    const QByteArray& GetCaptureData() const {
        return capture_data_;
    }

    void SetCapturePosition(std::optional<int> position) {
        capture_pos_ = position;
    }

    std::optional<int> GetCapturePosition() const {
        return capture_pos_;
    }

    void SetCaptureBytesNum(std::optional<int> num){
        capture_bytes_num_ = num;
    }

    std::optional<int> GetCaptureBytesNum(){
        return capture_bytes_num_;
    }

    void SetAnswerId(std::optional<quint32> id) {
        answer_id_ = id;
    }

    std::optional<quint32> GetAnswerId() const {
        return answer_id_;
    }

    void SetAnswerData(const QByteArray& byte_array) {
        answer_data_ = byte_array;
    }

    const QByteArray& GetAnswerData() const {
        return answer_data_;
    }

    QByteArray& GetAnswerData() {
        return answer_data_;
    }

    void SetAnswerRandomPosition(std::optional<int> position) {
        answer_random_pos_ = position;
    }

    std::optional<int> GetAnswerRandomPosition() const {
        return answer_random_pos_;
    }

    void SetAnswerRandomBytesNum(std::optional<int> bytes_num) {
        answer_random_bytes_num_ = bytes_num;
    }

    std::optional<int> GetAnswerRandomBytesNum() const {
        return answer_random_bytes_num_;
    }

    void SetAnswerRandomCheck(bool checked) {
        answer_random_check_ = checked;
    }

    bool GetAnswerRandomCheck() const {
        return answer_random_check_;
    }

    // Возвращает идентификатор и байты для захвата в виде QString.
    const QString GetCaptureString() const;

private:
    bool select_check_ = false;
    std::optional<quint32> capture_id_ = std::nullopt;
    QByteArray capture_data_;
    std::optional<int> capture_pos_ = std::nullopt;
    std::optional<int> capture_bytes_num_ = std::nullopt;
    std::optional<quint32> answer_id_ = std::nullopt;
    QByteArray answer_data_;
    std::optional<int> answer_random_pos_ = std::nullopt;
    std::optional<int> answer_random_bytes_num_ = std::nullopt;
    bool answer_random_check_ = false;
};

} // namespace emm

