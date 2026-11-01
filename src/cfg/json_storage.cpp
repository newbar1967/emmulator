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
    settings.GetValue().can_parameters.header_type = header_type;

    return true;
}

bool JsonStorage::LoadFilterFromJson(const QJsonArray& json_array, IdWithParameters& filter_by_id) {
    filter_by_id.clear();

    // Идем по массиву JSON.
    for (auto it = json_array.constBegin(); it != json_array.constEnd(); ++it) {
        const auto json_value = *it;
        quint32 id = 0;
        IdParameters id_parameters;
        if (!FromJsonToFilterId(json_value, id, id_parameters)) {
            return false; // Неправильный формат JSON.
        }

        filter_by_id[id] = id_parameters;
    }

    return true;
}

bool JsonStorage::LoadColorIdFromJson(const QJsonArray& json_array, IdWithParameters& color_by_id) {
    color_by_id.clear();

    // Идем по массиву JSON.
    for (auto it = json_array.constBegin(); it != json_array.constEnd(); ++it) {
        const auto json_value = *it;
        quint32 id = 0;
        IdParameters id_parameters;
        if (!FromJsonToColorId(json_value, id, id_parameters)) {
            return false; // Неправильный формат JSON.
        }

        color_by_id[id] = id_parameters;
    }

    return true;
}

bool JsonStorage::FromJsonToFilterId(const QJsonValue& json_value, quint32& id, IdParameters& id_parameters) {
    if (!json_value.isObject()) {
        return false; // Неправильный формат JSON.
    }

    const auto json_obj = json_value.toObject();

    // Проверим, содержатся ли нужные ключи.
    const QStringList checked_keys = {
        KEY_ANSWER_ID,
        KEY_HEADER_TYPE
    };
    if (!AreJsonKeysValid(json_obj, checked_keys)) {
        return false; // Неправильный формат JSON.
    }

    // Ключ "answer id".
    QString id_str;
    if (!FromJsonToString(json_obj,
                          KEY_ANSWER_ID,
                          id_str,
                          checkOption::HEX,
                          false)) {
        return false; // Неправильный формат JSON.
    }
    bool ok;
    id = id_str.toInt(&ok, 16);
    if (!ok) {
        return false; // Неправильный формат JSON.
    }

    // Ключ "header type".
    if (!FromJsonToString(json_obj,
                          KEY_HEADER_TYPE,
                          id_parameters.header_type,
                          checkOption::NONE,
                          false)) {
        return false; // Неправильный формат JSON.
    }
    // Проверим, есть ли такая строка в combobox.
    if (!settings_.GetCanHeaderTypeList().contains(id_parameters.header_type)) {
        return false; // Неправильный формат JSON.
    }
    if (id_parameters.header_type == EXTENDED_HEADER) { // Если заголовок расширенный,
        id |= EXTENDED_HEADER_ADD_BIT; // то взводим верхний бит ID.
    }

    return true;
}

bool JsonStorage::FromJsonToColorId(const QJsonValue& json_value, quint32& id, IdParameters& id_parameters) {
    if (!json_value.isObject()) {
        return false; // Неправильный формат JSON.
    }

    const auto json_obj = json_value.toObject();

    // Проверим, содержатся ли нужные ключи.
    const QStringList checked_keys = {
        KEY_ANSWER_ID,
        KEY_HEADER_TYPE,
        KEY_COLOR_RED,
        KEY_COLOR_GREEN,
        KEY_COLOR_BLUE
    };
    if (!AreJsonKeysValid(json_obj, checked_keys)) {
        return false; // Неправильный формат JSON.
    }

    // Ключ "answer id".
    QString id_str;
    if (!FromJsonToString(json_obj,
                          KEY_ANSWER_ID,
                          id_str,
                          checkOption::HEX,
                          false)) {
        return false; // Неправильный формат JSON.
    }
    bool ok;
    id = id_str.toInt(&ok, 16);
    if (!ok) {
        return false; // Неправильный формат JSON.
    }

    // Ключ "header type".
    if (!FromJsonToString(json_obj,
                          KEY_HEADER_TYPE,
                          id_parameters.header_type,
                          checkOption::NONE,
                          false)) {
        return false; // Неправильный формат JSON.
    }
    // Проверим, есть ли такая строка в combobox.
    if (!settings_.GetCanHeaderTypeList().contains(id_parameters.header_type)) {
        return false; // Неправильный формат JSON.
    }
    if (id_parameters.header_type == EXTENDED_HEADER) { // Если заголовок расширенный,
        id |= EXTENDED_HEADER_ADD_BIT; // то взводим верхний бит ID.
    }

    // Цвета.
    if (!json_obj.value(KEY_COLOR_RED).isDouble()
        || !json_obj.value(KEY_COLOR_GREEN).isDouble()
        || !json_obj.value(KEY_COLOR_BLUE).isDouble()
        ) {
        return false; // Неправильный формат JSON.
    }

    auto red = json_obj.value(KEY_COLOR_RED).toInt();
    if (red < 0 || red > 255) {
        return false; // Неправильный формат JSON.
    }
    auto green = json_obj.value(KEY_COLOR_GREEN).toInt();
    if (green < 0 || green > 255) {
        return false; // Неправильный формат JSON.
    }
    auto blue = json_obj.value(KEY_COLOR_BLUE).toInt();
    if (blue < 0 || blue > 255) {
        return false; // Неправильный формат JSON.
    }

    id_parameters.color = QColor(red, green, blue);
    if (!id_parameters.color.isValid()) {
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
    json_curr_obj.insert(KEY_HEADER_TYPE, header_type);

    json_obj.insert(KEY_CAN_PARAMETERS, json_curr_obj);
}

void JsonStorage::SaveFilterToJson(QJsonObject& json_obj, const QString key, IdWithParameters& filter_by_id) {
    QJsonArray json_array;

    for (const auto& [id, id_parameters] : filter_by_id) {
        QJsonObject json_curr_obj;
        // id.
        auto id_str = QString::number(id & EXTENDED_HEADER_SUB_BIT, 16).toUpper().rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
        json_curr_obj.insert(KEY_ANSWER_ID, id_str);
        // Header type.
        json_curr_obj.insert(KEY_HEADER_TYPE, id_parameters.header_type);

        json_array.append(json_curr_obj);
    }

    json_obj.insert(key, json_array);
}

void JsonStorage::SaveColorIdToJson(QJsonObject& json_obj, const QString key, IdWithParameters& color_by_id) {
    QJsonArray json_array;

    for (const auto& [id, id_parameters] : color_by_id) {
        QJsonObject json_curr_obj;
        // id.
        auto id_str = QString::number(id & EXTENDED_HEADER_SUB_BIT, 16).toUpper().rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
        json_curr_obj.insert(KEY_ANSWER_ID, id_str);
        // Header type.
        json_curr_obj.insert(KEY_HEADER_TYPE, id_parameters.header_type);
        // Color.
        json_curr_obj.insert(KEY_COLOR_RED, id_parameters.color.red());
        json_curr_obj.insert(KEY_COLOR_GREEN, id_parameters.color.green());
        json_curr_obj.insert(KEY_COLOR_BLUE, id_parameters.color.blue());
        json_array.append(json_curr_obj);
    }

    json_obj.insert(key, json_array);
}

void JsonStorage::SaveMessageColoringModeToJson(QJsonObject& json_obj, const QString key, bool& mode) {
   json_obj.insert(key, mode);
}

std::optional<QString> JsonStorage::GetConfigPath() {
    // Папка Roaming: %APPDATA%
    QString base_dir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    // Создаем подпапку для приложения.
    const QString folder_name = QStringLiteral("ecu_emmulator");
    QDir dir(base_dir);
    // mkpath принимает относительный путь — разделители подставит сам.
    if (!dir.mkpath(folder_name)) {
        return std::nullopt; // Не удалось создать папку.
    }

    // Возвращаем абсолютный путь к папке — уже с правильным разделителем.
    return dir.absoluteFilePath(folder_name);
}

} // namespace cfg

