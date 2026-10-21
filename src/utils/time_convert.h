#pragma once

#include <stdint.h>

inline constexpr int MILLISECONDS_PER_SECOND = 1000;
inline constexpr int SECONDS_IN_HOUR = 3600;
inline constexpr int MINUTES_IN_HOUR = 60;

class QString;

namespace utils {

class TimeConvert {
public:
    enum class convertType {
        UNKNOWN,
        HH_MM_SS_MS,
        MMM_SS_MS,
        SSSS_MS
    };

    TimeConvert() = default;

    TimeConvert(convertType convert_type)
        : convert_type_(convert_type) {
    }

    QString MsToString(uint32_t timeMs);

private:
    QString MsToString_hh_mm_ss_ms(uint32_t timeMs);
    QString MsToString_mmm_ss_ms(uint32_t timeMs);
    QString MsToString_ssss_ms(uint32_t timeMs);

    convertType convert_type_ = convertType::UNKNOWN;
};

} // namespace utils
