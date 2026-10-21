#include <QString>

#include "time_convert.h"

namespace utils {

QString TimeConvert::MsToString(uint32_t timeMs) {
    switch (convert_type_) {
    case convertType::HH_MM_SS_MS:
        return MsToString_hh_mm_ss_ms(timeMs);
    case convertType::MMM_SS_MS:
        return MsToString_mmm_ss_ms(timeMs);
    case convertType::SSSS_MS:
        return MsToString_ssss_ms(timeMs);
    case convertType::UNKNOWN:
        break;
    }

    return {};
}

QString TimeConvert::MsToString_hh_mm_ss_ms(uint32_t timeMs) {
    uint32_t total_sec = timeMs / MILLISECONDS_PER_SECOND;
    uint32_t hour = total_sec / SECONDS_IN_HOUR;
    uint32_t min = (total_sec - hour * SECONDS_IN_HOUR) / MINUTES_IN_HOUR;
    uint32_t sec = total_sec - hour * SECONDS_IN_HOUR - min * MINUTES_IN_HOUR;
    uint32_t ms = timeMs % 1000;
    return(QString("%1:%2:%3.%4")
                .arg(hour, 2, 10, QChar('0'))
                .arg(min, 2, 10, QChar('0'))
                .arg(sec, 2, 10, QChar('0'))
                .arg(ms, 3, 10, QChar('0')));
}

QString TimeConvert::MsToString_mmm_ss_ms(uint32_t timeMs) {
    uint32_t total_sec = timeMs / MILLISECONDS_PER_SECOND;
    uint32_t ms = timeMs % MILLISECONDS_PER_SECOND;
    uint32_t min = total_sec / MINUTES_IN_HOUR;
    uint32_t sec = total_sec % MINUTES_IN_HOUR;
    return(QString("%1:%2.%3")
                .arg(min, 3, 10, QChar('0'))
                .arg(sec, 2, 10, QChar('0'))
                .arg(ms, 3, 10, QChar('0')));
}

QString TimeConvert::MsToString_ssss_ms(uint32_t timeMs) {
    uint32_t total_sec = timeMs / MILLISECONDS_PER_SECOND;
    uint32_t ms = timeMs % MILLISECONDS_PER_SECOND;
    return(QString("%1.%2")
                .arg(total_sec, 4, 10, QChar('0'))
                .arg(ms, 3, 10, QChar('0')));
}

} // namespace utils
