#include "capture_answer_model.h"

namespace emm {

CaptureAnswerModel::CaptureAnswerModel(QObject* parent)
    : QObject(parent)
{}

// Возвращает идентификатор и байты для захвата в виде единой QString.
// Для текущего механизма сравнения есть ограничение по длине - максимум 8 байт.
// И байта для сравнения должны начинаться с позиции 0.
const QString CaptureAnswerModel::GetCaptureString() const
{
    QByteArray result;
    QDataStream out(&result, QIODevice::WriteOnly);
    // Задаём порядок байтов.
    out.setByteOrder(QDataStream::LittleEndian);

    // Сначала проверки.
    if (!capture_id_.has_value()
        || !capture_pos_.has_value()
        || !capture_bytes_num_.has_value()
        || capture_data_.isEmpty()) {
        return {}; // Пустая строка.
    }

    quint32 id = capture_id_.value();
    int pos = capture_pos_.value();
    int bytes_num = capture_bytes_num_.value();

    // Позиция не должна выходить за пределы массива.
    if (pos < 0 || pos >= capture_data_.size()) {
        return {}; // Пустая строка.
    }

    // Должно быть количество байт для захвата.
    if (bytes_num == 0) {
        return {}; // Пустая строка.
    }

    // Записываем идентификатор в поток.
    out << id;

    // Определяем количество байт, которые реально надо извлечь.
    int available_bytes = capture_data_.size() - pos;
    int actual_bytes_to_copy = std::min(bytes_num, available_bytes);
    // Копируем указанные байты из capture_data_ в выходной поток.
    out.writeRawData(capture_data_.constData() + pos, actual_bytes_to_copy);

    return QByteArrayToQString(result, utils::stringOption::WITHOUT_SPACES);
}

} // namespace emm
