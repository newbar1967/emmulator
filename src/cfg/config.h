#pragma once

#include <QString>
#include <QVector>
#include <QColor>

namespace cfg {

struct CaptureAnswerData {
    bool select = false;

    QString comment;

    QString capture_id;
    QString capture_data;
    QString capture_pos;
    QString capture_bytes_num;

    QString answer_id;
    QString answer_data;
    QString answer_random_pos;
    QString answer_random_bytes_num;
    bool answer_random_check = false;
};

class Config final
{
    using CallBack = std::function<void()>;
    using CallBackVector = QVector<CallBack>;

public:
    using CaptureAnswerDatas = std::vector<CaptureAnswerData>;

    Config() = default;
    Config(Config& other) = delete;
    Config& operator=(Config& other) = delete;
    Config(Config&& other) = delete;
    Config& operator=(Config&& other) = delete;

    void Update();

    void AddCallBack(CallBack cb) {
        if (cb != nullptr) {
            cbs_.push_back(cb);
        }
    }

public:
    CaptureAnswerDatas capture_answer_datas;

private:
    CallBackVector cbs_; // Массив функциональных объектов.
};

} // namespace cfg

