#pragma once

#include <QString>

namespace string_converter {

template<typename T>
inline T to(const QString& str, bool* ok, int base = 10) {
    // Общая реализация — не поддерживается.
    static_assert(sizeof(T) == 0, "Conversion to this type is not supported");
    return T();
}

// Специализация для int
template<>
inline int to<int>(const QString& str, bool* ok, int base) {
    return str.toInt(ok, base);
}

// Специализация для quint32
template<>
inline quint32 to<quint32>(const QString& str, bool* ok, int base) {
    return static_cast<quint32>(str.toUInt(ok, base));
}

} // namespace string_converter

