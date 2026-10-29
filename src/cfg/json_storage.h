#pragma once

#include <QString>
#include <optional>
#include "storage.h"
#include "settings.h"
#include "config.h"
#include "common_cfg.h"

namespace cfg {

inline static const QString KEY_SELECT = "select";
inline static const QString KEY_CAPTURE_ANSWERS = "capture answers";
inline static const QString KEY_COMMENT = "comment";
inline static const QString KEY_CAPTURE_ID = "capture id";
inline static const QString KEY_CAPTURE_DATA = "capture data";
inline static const QString KEY_CAPTURE_POS = "capture pos";
inline static const QString KEY_CAPTURE_BYTES_NUM = "capture bytes num";
inline static const QString KEY_ANSWER_ID = "answer id";
inline static const QString KEY_ANSWER_DATA = "answer data";
inline static const QString KEY_ANSWER_RANDOM_POS = "answer random pos";
inline static const QString KEY_ANSWER_RANDOM_BYTES_NUM = "answer random bytes num";
inline static const QString KEY_ANSWER_RANDOM_CHECK = "answer random check";

inline static const QString KEY_CAN_PARAMETERS = "can parameters";
inline static const QString KEY_BAUD_RATE = "baud rate";
inline static const QString KEY_HEADER_TYPE = "header type";

inline static const QString KEY_SOFT_FILTER = "soft filter";
inline static const QString KEY_HARD_FILTER = "hard filter";

inline static const QString KEY_COLOR_ID = "color id";
inline static const QString KEY_MESSAGE_COLORORING_MODE = "message coloring mode";

inline static const QString KEY_COLOR_RED = "color red";
inline static const QString KEY_COLOR_GREEN = "color green";
inline static const QString KEY_COLOR_BLUE = "color blue";

class JsonStorage : public Storage {
public:
    JsonStorage(cfg::Settings& settings, cfg::Config& config)
        : settings_(settings)
        , config_(config)
    {}

    bool Load(const QString& full_file_name) override;
    bool Save(const QString& full_file_name) override;
    bool LoadViaFileDialog() override;
    void SaveViaFileDialog() override;
    std::optional<QString> GetConfigPath();

private:
    enum class checkOption {
        NONE,
        HEX,
        INT,
        BYTES,
        DOUBLE
    };

    bool FromJsonToBool(const QJsonObject& obj, const QString& key, bool& field);
    bool FromJsonToString(const QJsonObject& obj,
                          const QString& key,
                          QString& field,
                          checkOption option = checkOption::NONE,
                          bool check_empty = true);
    bool AreJsonKeysValid(const QJsonObject& json_obj, const QStringList& keys);
    bool LoadCaptureAnswersFromJson(const QJsonArray& json_array, Config::CaptureAnswerDatas& capture_answer_datas);
    bool LoadCanParametersFromJson(const QJsonObject& json_obj, Settings& settings);
    bool LoadFilterFromJson(const QJsonArray& json_array, IdWithParameters& filter_by_id);
    bool LoadColorIdFromJson(const QJsonArray& json_array, IdWithParameters& color_by_id);
    bool FromJsonToColorId(const QJsonValue& json_value, quint32& id, IdParameters& id_parameters);
    bool FromJsonToFilterId(const QJsonValue& json_value, quint32& id, IdParameters& id_parameters);
    void SaveCaptureAnswersToJson(QJsonObject& json_obj);
    void SaveCanParametersToJson(QJsonObject& json_obj);
    void SaveFilterToJson(QJsonObject& json_obj, const QString key, IdWithParameters& filter_by_id);
    void SaveColorIdToJson(QJsonObject& json_obj, const QString key, IdWithParameters& color_by_id);
    void SaveMessageColoringModeToJson(QJsonObject& json_obj, const QString key, bool& mode);

private:
    cfg::Settings& settings_;
    cfg::Config& config_;
};

} // namespace cfg
