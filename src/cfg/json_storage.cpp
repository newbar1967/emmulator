#include <QtGlobal>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <QColor>
#include <QStandardPaths>

#include "utils.h"
#include "json_storage.h"

namespace cfg {

bool JsonStorage::LoadViaFileDialog() {
    QFileDialog file_dialog;

    file_dialog.setDirectory(settings_.LoadCfgDirectory()); // Вспомним директорию для файлов конфигурации.

    QString full_file_name = file_dialog.getOpenFileName(QApplication::activeWindow(),
                                                         "Чтение конфигурации из файла",
                                                         "",
                                                         "*.json");
    QFileInfo fileInfo(full_file_name);
    QString file_name = fileInfo.fileName();

    if (full_file_name.isEmpty()) {
        return false; // Если нажата "Отмена", то просто выходим.
    }

    if (!Load(full_file_name)) {
        QMessageBox::critical(QApplication::activeWindow(),
                              "Ошибка",
                              "Ошибка загрузки конфигурации из файла " + file_name);
        return false;
    }

    settings_.SaveCfgDirectory(fileInfo.absolutePath()); // Сохраняем директорию для файлов конфигурации.

//    QMessageBox::information(QApplication::activeWindow(),
//                             "Информация",
//                             "Конфигурация загружена из файла " + file_name);

    return true;
}

void JsonStorage::SaveViaFileDialog() {
    QFileDialog file_dialog;

    file_dialog.setDirectory(settings_.LoadCfgDirectory()); // Вспомним директорию для файлов конфигурации.

    QString full_file_name = file_dialog.getSaveFileName(QApplication::activeWindow(),
                                                         "Сохранение конфигурации в файл",
                                                         "",
                                                         "*.json");
    QFileInfo fileInfo(full_file_name);
    QString file_name = fileInfo.fileName();

    if (full_file_name.isEmpty()) {
        return;  // Если нажата "Отмена", то просто выходим.
    }

    if (!Save(full_file_name)) {
        QMessageBox::critical(QApplication::activeWindow(),
                              "Ошибка",
                              "Ошибка сохранения конфигурации в файл " + file_name);
        return;
    }

    settings_.SaveCfgDirectory(fileInfo.absolutePath()); // Сохраняем директорию для файлов конфигурации.

//    QMessageBox::information(QApplication::activeWindow(),
//                             "Информация",
//                             "Конфигурация сохранена в файл " + file_name);
}

bool JsonStorage::Load(const QString& full_file_name) {
    QFileInfo file_info(full_file_name);
    if ((!file_info.exists() || !file_info.isFile())) {
        return false; // Ошибка открытия файла конфигурации.
    }

    // Открываем файл для чтения.
    QFile file(full_file_name);
    if(file.open(QIODevice::ReadOnly) == false) {
        return false; // Ошибка открытия файла конфигурации.
    }

    // Читаем файл.
    QByteArray data;
    data = file.readAll();
    file.close();

    auto json_doc = QJsonDocument::fromJson(data);

    // Если не json object.
    if (!json_doc.isObject()) {
        return false; // Неправильный формат JSON.
    }

    auto json_top_obj = json_doc.object();
    if (json_top_obj.isEmpty()) {
        return false; // Файл не содержит JSON-объектов.
    }

    // Проверим, содержатся ли нужные ключи верхнего уровня.
    const QStringList checked_keys = {
        KEY_CAPTURE_ANSWERS,
        KEY_CAN_PARAMETERS,
        KEY_SOFT_FILTER,
        KEY_HARD_FILTER,
        KEY_COLOR_ID,
        KEY_MESSAGE_COLORORING_MODE
    };
    if (!AreJsonKeysValid(json_top_obj, checked_keys)) {
        return false; // Неправильный формат JSON.
    }

    // Вытаскиваем данные объектов capture answers.
    Config::CaptureAnswerDatas tmp_capture_answer_datas;
    {
        auto json_array = json_top_obj[KEY_CAPTURE_ANSWERS].toArray();
        if (!LoadCaptureAnswersFromJson(json_array, tmp_capture_answer_datas)) {
            return false; // Неправильный формат JSON.
        }
    }

    // Вытаскиваем все настройки.
    cfg::Settings tmp_settings;

    // Настройки CAN.
    {
        auto json_obj = json_top_obj[KEY_CAN_PARAMETERS].toObject();
        if (!LoadCanParametersFromJson(json_obj, tmp_settings)) {
            return false; // Неправильный формат JSON.
        }
    }

    // Настройки программного фильтра.
    {
        auto json_array = json_top_obj[KEY_SOFT_FILTER].toArray();
        if (!LoadFilterFromJson(json_array, tmp_settings.GetValue().soft_filter_by_id)) {
            return false; // Неправильный формат JSON.
        }
    }

    // Настройки аппаратного фильтра.
    {
        auto json_array = json_top_obj[KEY_HARD_FILTER].toArray();
        if (!LoadFilterFromJson(json_array, tmp_settings.GetValue().hard_filter_by_id)) {
            return false; // Неправильный формат JSON.
        }
    }

    // Подкраска сообщений по id.
    {
        auto json_array = json_top_obj[KEY_COLOR_ID].toArray();
        if (!LoadColorIdFromJson(json_array, tmp_settings.GetValue().color_by_id)) {
            return false; // Неправильный формат JSON.
        }
    }

    // Подкрашивать или нет сообщения.
    {
        if (!FromJsonToBool(json_top_obj, KEY_MESSAGE_COLORORING_MODE, tmp_settings.GetValue().message_coloring_mode)) {
            return false; // Неправильный формат JSON.
        }
    }

    // Если прошло успешно, переносим данные конфигурации и настроек.
    config_.capture_answer_datas = std::move(tmp_capture_answer_datas);
    settings_.GetValue() = tmp_settings.GetValue();

    return true;
}

bool JsonStorage::LoadCaptureAnswersFromJson(const QJsonArray& json_array, Config::CaptureAnswerDatas& capture_answer_datas) {
    // Идем по массиву JSON.
    for (auto it = json_array.constBegin(); it != json_array.constEnd(); ++it) {
        const auto json_obj = *it;
        // Только, если это JSON-объект.
        if (!json_obj.isObject()) {
            return false; // Неправильный формат JSON.
        }

        // Проверим значения по ключам.
        const QStringList checked_keys = {
            KEY_SELECT,
            KEY_COMMENT,
            KEY_CAPTURE_ID,
            KEY_CAPTURE_DATA,
            KEY_CAPTURE_POS,
            KEY_CAPTURE_BYTES_NUM,
            KEY_ANSWER_ID,
            KEY_ANSWER_DATA,
            KEY_ANSWER_RANDOM_POS,
            KEY_ANSWER_RANDOM_BYTES_NUM,
            KEY_ANSWER_RANDOM_CHECK
        };
        if (!AreJsonKeysValid(json_obj.toObject(), checked_keys)) {
            return false; // Неправильный формат JSON.
        }

        // Данные для объекта захвата и ответа.
        CaptureAnswerData capture_answer_data;

        // Обновляем поля из файла.
        // Ключ "select".
        if (!FromJsonToBool(json_obj.toObject(),
                            KEY_SELECT,
                            capture_answer_data.select)) {
            return false; // Неправильный формат JSON.
        }

        // Ключ "comment".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_COMMENT,
                              capture_answer_data.comment)) {
            return false; // Неправильный формат JSON.
        }

        // Capture.
        // Ключ "capture id".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_CAPTURE_ID,
                              capture_answer_data.capture_id,
                              checkOption::HEX,
                              false)) {
            return false; // Неправильный формат JSON.
        }
        // Ключ "capture data".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_CAPTURE_DATA,
                              capture_answer_data.capture_data,
                              checkOption::BYTES)) {
            return false; // Неправильный формат JSON.
        }
        // Ключ "capture pos".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_CAPTURE_POS,
                              capture_answer_data.capture_pos,
                              checkOption::INT,
                              false)) {
            return false; // Неправильный формат JSON.
        }
        // Внимание! Для упрощения и ускорения процедуры захвата
        // принудительно в поле capture_pos записываем "0"
        capture_answer_data.capture_pos = "0";

        // Ключ "capture bytes num".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_CAPTURE_BYTES_NUM,
                              capture_answer_data.capture_bytes_num,
                              checkOption::INT,
                              false)) {
            return false; // Неправильный формат JSON.
        }

        // Answer.
        // Ключ "answer id".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_ANSWER_ID,
                              capture_answer_data.answer_id,
                              checkOption::HEX,
                              false)) {
            return false; // Неправильный формат JSON.
        }
        // Ключ "answer data".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_ANSWER_DATA,
                              capture_answer_data.answer_data,
                              checkOption::BYTES)) {
            return false; // Неправильный формат JSON.
        }
        // Ключ "answer random pos".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_ANSWER_RANDOM_POS,
                              capture_answer_data.answer_random_pos,
                              checkOption::INT,
                              false)) {
            return false; // Неправильный формат JSON.
        }
        // Ключ "answer random bytes num".
        if (!FromJsonToString(json_obj.toObject(),
                              KEY_ANSWER_RANDOM_BYTES_NUM,
                              capture_answer_data.answer_random_bytes_num,
                              checkOption::INT,
                              false)) {
            return false; // Неправильный формат JSON.
        }
        // Ключ "answer random check".
        if (!FromJsonToBool(json_obj.toObject(),
                            KEY_ANSWER_RANDOM_CHECK,
                            capture_answer_data.answer_random_check)) {
            return false; // Неправильный формат JSON.
        }

        capture_answer_datas.push_back(capture_answer_data);
    }

    return true;
}

bool JsonStorage::LoadCanParametersFromJson(const QJsonObject& json_obj, Settings& settings) {
    // Проверим, содержатся ли нужные ключи.
    const QStringList checked_keys = {
        KEY_BAUD_RATE,
        KEY_HEADER_TYPE
    };
    if (!AreJsonKeysValid(json_obj, checked_keys)) {
        return false; // Неправильный формат JSON.
    }

    // Ключ "baud rate".
    QString baud_rate;
    if (!FromJsonToString(json_obj,
                          KEY_BAUD_RATE,
                          baud_rate,
                          checkOption::DOUBLE,
                          false)) {
        return false; // Неправильный формат JSON.
    }
    // Проверим, есть ли такая строка в combobox.
    if (!settings.GetCanBaudRateList().contains(baud_rate)) {
        return false; // Неправильный формат JSON.
    }
    // Теперь можно скопировать.
    settings.GetValue().can_parameters.baud_rate = baud_rate.toDouble() * cfg::Settings::BAUD_RATE_COEF;

    // Ключ "header type".
    QString header_type;
    if (!FromJsonToString(json_obj,
                          KEY_HEADER_TYPE,
                          header_type,
                          checkOption::NONE,
                          false)) {
        return false; // Неправильный формат JSON.
    }
    // Проверим, есть ли такая строка в combobox.
    if (!settings.GetCanHeaderTypeList().contains(header_type)) {
        return false; // Неправильный формат JSON.
    }
    // Теперь можно скопировать.
    settings.GetValue().can_parameters.header_type
        = settings.GetCanHeaderTypeList().indexOf(header_type);

    return true;
}

bool JsonStorage::LoadFilterFromJson(const QJsonArray& json_array, std::unordered_set<quint32>& filter_by_id) {
    filter_by_id.clear();

    // Идем по массиву JSON.
    for (auto it = json_array.constBegin(); it != json_array.constEnd(); ++it) {
        const auto json_value = *it;

        // Извлекаем очередной id.
        quint32 id;
        if (!FromJsonToId(json_value, id)) {
            return false; // Неправильный формат JSON.
        }

        // Записываем id в список для фильтрации.
        filter_by_id.insert(id);
    }

    return true;
}

bool JsonStorage::LoadColorIdFromJson(const QJsonArray& json_array, std::unordered_map<quint32, QColor>& color_by_id) {
    color_by_id.clear();

    // Идем по массиву JSON.
    for (auto it = json_array.constBegin(); it != json_array.constEnd(); ++it) {
        const auto json_value = *it;
        if (!json_value.isArray() && json_value.toArray().size() != 4) {
            return false; // Неправильный формат JSON.
        }

        quint32 id = 0;
        QColor color = QColor(255, 255, 255);
        if (!FromJsonToIdAndColor(json_value.toArray(), id, color)) {
            return false; // Неправильный формат JSON.
        }

        color_by_id[id] = color;
    }

    return true;
}

bool JsonStorage::FromJsonToId(const QJsonValue& json_value, quint32& id) {
    if (!json_value.isString()) {
        return false; // Неправильный формат JSON.
    }

    auto str = json_value.toString();

    if (!utils::IsHexString(str)
        || str.size() > cfg::SettingsWindow::CAN_ID_MAX_LENGTH) { // Символов должно быть не больше 8.
        return false; // Неправильный формат JSON.
    }

    bool ok;
    id = str.toInt(&ok, 16);

    return true;
}

bool JsonStorage::FromJsonToColorComponent(const QJsonValue& json_value, int& component) {
    if (!json_value.isDouble()) {
        return false; // Неправильный формат JSON.
    }

    auto tmp = json_value.toInt();
    if (tmp < 0 || tmp > 255) {
        return false; // Неправильный формат JSON.
    }

    component = tmp;
    return true;
}

bool JsonStorage::FromJsonToIdAndColor(const QJsonArray& json_array, quint32& id, QColor& color) {
    // Извлекаем id.
    if (!FromJsonToId(json_array.at(0), id)) {
        return false; // Неправильный формат JSON.
    }

    int red = 0;
    int green = 0;
    int blue = 0;;

    // Цвет red.
    if (!FromJsonToColorComponent(json_array.at(1), red)) {
        return false; // Неправильный формат JSON.
    }

    // Цвет green.
    if (!FromJsonToColorComponent(json_array.at(2), green)) {
        return false; // Неправильный формат JSON.
    }

    // Цвет blue.
    if (!FromJsonToColorComponent(json_array.at(3), blue)) {
        return false; // Неправильный формат JSON.
    }

    color = QColor(red, green, blue);
    if (!color.isValid()) {
        return false;
    }

    return true;
}

bool JsonStorage::FromJsonToBool(const QJsonObject& obj, const QString& key, bool& field) {
    if (!obj[key].isBool()) {
        return false; // Неправильный формат JSON.
    }
    field = obj[key].toBool();
    return true;
}

bool JsonStorage::FromJsonToString(const QJsonObject& obj,
                                   const QString& key,
                                   QString& field,
                                   checkOption option,
                                   bool check_empty)
{
    if (!obj[key].isString()) {
        return false; // Неправильный формат JSON.
    }

    field = obj[key].toString();

    if (check_empty && field.isEmpty()) { // Если надо, чтобы пустая строка прошла проверку.
        return true; // Пустая строка допускается.
    }

    if (option == checkOption::INT && !utils::IsIntString(field)) {
        return false;
    } else if (option == checkOption::DOUBLE && !utils::IsDoubleString(field)) {
        return false;
    } else if (option == checkOption::BYTES) {
        if (!utils::CheckTextBytesFormat(field)) {
            return false;
        } else {
            // Пребразовываем в верхний регистр.
            field = field.toUpper();
        }
    } else if (option == checkOption::HEX) {
        if (!utils::IsHexString(field)) {
            return false;
        } else {
            // Пребразовываем в верхний регистр.
            field = field.toUpper();
        }
    } else if (field.isEmpty()) {
        return false;
    }

    return true;
}

bool JsonStorage::AreJsonKeysValid(const QJsonObject& json_obj, const QStringList& keys) {
    for (const QString& key : keys) {
        // Проверяем наличие ключа.
        if (!json_obj.contains(key)) {
            return false;
        }

        // Проверяем валидность значения.
        const QJsonValue value = json_obj[key];
        if (value.isNull() || value.isUndefined()) {
            return false;
        }
    }
    return true;
}

bool JsonStorage::Save(const QString& full_file_name) {
    // Открываем файл для записи.
    QFile file(full_file_name);
    if(file.open(QIODevice::WriteOnly) == false) {
        return false; // Ошибка открытия файла для записи конфигурации.
    }

    // Создание JSON документа.
    // На верхнем уровне json-объект, состоящий из json-объектов.
    QJsonObject json_top_obj;

    // Объекты захвата и ответа.
    SaveCaptureAnswersToJson(json_top_obj);

    // CAN параметры.
    SaveCanParametersToJson(json_top_obj);

    // Программный фильтр.
    SaveFilterToJson(json_top_obj, KEY_SOFT_FILTER, settings_.GetValue().soft_filter_by_id);

    // Аппаратный фильтр.
    SaveFilterToJson(json_top_obj, KEY_HARD_FILTER, settings_.GetValue().hard_filter_by_id);

    // Подкраска сообщений.
    SaveColorIdToJson(json_top_obj, KEY_COLOR_ID, settings_.GetValue().color_by_id);

    // Подкрашивать или нет сообщения.
    SaveMessageColoringModeToJson(json_top_obj, KEY_MESSAGE_COLORORING_MODE, settings_.GetValue().message_coloring_mode);

    QJsonDocument doc(json_top_obj);
    QByteArray data = doc.toJson();

    // Записываем файл.
    if(file.write(data, data.size()) != data.size()) {
        file.close();
        return false; // Ошибка записи файла конфигурации.
    }
    file.close();
    return true;
}

void JsonStorage::SaveCaptureAnswersToJson(QJsonObject& json_obj) {
    QJsonArray json_array;

    QSet<QString> capture_datas; // Для предотвращения дублирования объектов.
    capture_datas.reserve(config_.capture_answer_datas.size());

    for (size_t i = 0; i < config_.capture_answer_datas.size(); ++i) {
        // Объекты с дублирующими строками не сохраняем.
        if (!config_.capture_answer_datas[i].capture_data.isEmpty() // Проверяем, только если строка непустая.
            && capture_datas.contains(config_.capture_answer_datas[i].capture_data)) {
            // Такая строка уже есть, пропускаем.
            continue;
        }

        const auto capture_answer_data = config_.capture_answer_datas[i];

        // Создаем текущий элемент списка.
        QJsonObject json_curr_obj;

        // Select.
        json_curr_obj.insert(KEY_SELECT, capture_answer_data.select);
        // Comment.
        json_curr_obj.insert(KEY_COMMENT, capture_answer_data.comment);
        // Capture.
        json_curr_obj.insert(KEY_CAPTURE_ID, capture_answer_data.capture_id);
        json_curr_obj.insert(KEY_CAPTURE_DATA, capture_answer_data.capture_data);
        if (!config_.capture_answer_datas[i].capture_data.isEmpty()) {
            capture_datas.insert(capture_answer_data.capture_data); // Для последующего сравнения.
        }
        json_curr_obj.insert(KEY_CAPTURE_POS, capture_answer_data.capture_pos);
        json_curr_obj.insert(KEY_CAPTURE_BYTES_NUM, capture_answer_data.capture_bytes_num);
        // Answer.
        json_curr_obj.insert(KEY_ANSWER_ID, capture_answer_data.answer_id);
        json_curr_obj.insert(KEY_ANSWER_DATA, capture_answer_data.answer_data);
        json_curr_obj.insert(KEY_ANSWER_RANDOM_POS, capture_answer_data.answer_random_pos);
        json_curr_obj.insert(KEY_ANSWER_RANDOM_BYTES_NUM, capture_answer_data.answer_random_bytes_num);
        json_curr_obj.insert(KEY_ANSWER_RANDOM_CHECK, capture_answer_data.answer_random_check);

        // Добавляем элемент в список json.
        json_array.append(json_curr_obj);
    }

    json_obj.insert(KEY_CAPTURE_ANSWERS, json_array);
}

void JsonStorage::SaveCanParametersToJson(QJsonObject& json_obj) {
    QJsonObject json_curr_obj;

    // Baud rate.
    auto baud_rate = settings_.GetValue().can_parameters.baud_rate / cfg::Settings::BAUD_RATE_COEF;
    json_curr_obj.insert(KEY_BAUD_RATE, QString::number(baud_rate));
    // Header type.
    auto header_type = settings_.GetValue().can_parameters.header_type;
    json_curr_obj.insert(KEY_HEADER_TYPE, settings_.GetCanHeaderTypeList().at(header_type));

    json_obj.insert(KEY_CAN_PARAMETERS, json_curr_obj);
}

void JsonStorage::SaveFilterToJson(QJsonObject& json_obj, const QString key, std::unordered_set<quint32>& filter_by_id) {
    QJsonArray json_array;

    for (const auto& id : filter_by_id) {
        auto id_str = QString::number(id, 16).toUpper().rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
        json_array.append(id_str);
    }

    json_obj.insert(key, json_array);
}

void JsonStorage::SaveColorIdToJson(QJsonObject& json_obj, const QString key, std::unordered_map<quint32, QColor>& color_by_id) {
    QJsonArray json_array;

    for (const auto& [id, color] : color_by_id) {
        QJsonArray json_curr_array;
        auto id_str = QString::number(id, 16).toUpper().rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
        json_curr_array.append(id_str);
        json_curr_array.append(color.red());
        json_curr_array.append(color.green());
        json_curr_array.append(color.blue());
        json_array.append(json_curr_array);
    }

    json_obj.insert(key, json_array);
}

void JsonStorage::SaveMessageColoringModeToJson(QJsonObject& json_obj, const QString key, bool& mode) {
   json_obj.insert(key, mode);
}

std::optional<QString> JsonStorage::GetConfigPath() {
    // Папка Roaming: %APPDATA%
    QString base_dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QString app_dir = base_dir + "/ECU Emmulator";

    // Создаем подпапку для приложения.
    QDir dir;
    if (!dir.mkpath(app_dir)) {
        return std::nullopt; // Не удалось создать папку.
    }

    return app_dir;
}

} // namespace cfg

