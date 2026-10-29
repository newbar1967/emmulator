#pragma once

#include <QColor>
#include <unordered_map>

namespace cfg {

struct IdParameters {
    QString header_type;
    QColor color = QColor(Qt::white);
};

// Для хранения идентификатора и соответствующих параметров
// Если идентификатор расширенный, то верхний бит ключа взводится
// для придания уникальности.
using IdWithParameters = std::unordered_map<quint32, IdParameters>;

inline static const QString STANDART_HEADER = "стандартный";
inline static const QString EXTENDED_HEADER = "расширенный";

inline constexpr quint32 STANDART_HEADER_MASK = 0x000007ffu;
inline constexpr quint32 EXTENDED_HEADER_MASK = 0x1fffffffu;
inline constexpr quint32 EXTENDED_HEADER_ADD_BIT = 0x80000000u;
inline constexpr quint32 EXTENDED_HEADER_SUB_BIT = 0x7fffffffu;

}
