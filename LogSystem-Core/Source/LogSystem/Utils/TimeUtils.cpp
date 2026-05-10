#include "LogSystem/Utils/TimeUtils.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace logsystem {
namespace {

bool IsDigit(char ch) {
    return ch >= '0' && ch <= '9';
}

bool ParseFixedInt(std::string_view text, int& value) {
    if (text.empty())
        return false;

    int parsed = 0;
    for (char ch : text) {
        if (!IsDigit(ch))
            return false;

        parsed = (parsed * 10) + (ch - '0');
    }

    value = parsed;
    return true;
}

} // namespace

std::string FormatTimestampToIso8601(TimestampUtc timestamp) {
    const auto dayPoint = std::chrono::floor<std::chrono::days>(timestamp);
    const std::chrono::year_month_day date { dayPoint };
    const std::chrono::hh_mm_ss time { timestamp - dayPoint };

    std::ostringstream output;
    output << std::setfill('0')
        << std::setw(4) << static_cast<int>(date.year()) << '-'
        << std::setw(2) << static_cast<unsigned>(date.month()) << '-'
        << std::setw(2) << static_cast<unsigned>(date.day()) << 'T'
        << std::setw(2) << time.hours().count() << ':'
        << std::setw(2) << time.minutes().count() << ':'
        << std::setw(2) << time.seconds().count() << '.'
        << std::setw(3) << std::chrono::duration_cast<std::chrono::milliseconds>(time.subseconds()).count()
        << 'Z';

    return output.str();
}

std::optional<TimestampUtc> ParseTimestampFromIso8601(std::string_view text) {
    if (text.size() != 24)
        return std::nullopt;

    if (text[4] != '-' || text[7] != '-' || text[10] != 'T' ||
        text[13] != ':' || text[16] != ':' || text[19] != '.' ||
        text[23] != 'Z') {
        return std::nullopt;
    }

    int yearValue = 0;
    int monthValue = 0;
    int dayValue = 0;
    int hourValue = 0;
    int minuteValue = 0;
    int secondValue = 0;
    int millisecondValue = 0;

    if (!ParseFixedInt(text.substr(0, 4), yearValue) ||
        !ParseFixedInt(text.substr(5, 2), monthValue) ||
        !ParseFixedInt(text.substr(8, 2), dayValue) ||
        !ParseFixedInt(text.substr(11, 2), hourValue) ||
        !ParseFixedInt(text.substr(14, 2), minuteValue) ||
        !ParseFixedInt(text.substr(17, 2), secondValue) ||
        !ParseFixedInt(text.substr(20, 3), millisecondValue)) {
        return std::nullopt;
    }

    const std::chrono::year_month_day date {
        std::chrono::year { yearValue },
        std::chrono::month { static_cast<unsigned>(monthValue) },
        std::chrono::day { static_cast<unsigned>(dayValue) }
    };

    if (!date.ok())
        return std::nullopt;

    if (hourValue < 0 || hourValue > 23 ||
        minuteValue < 0 || minuteValue > 59 ||
        secondValue < 0 || secondValue > 59) {
        return std::nullopt;
    }

    const auto days = std::chrono::sys_days { date };
    const auto timestamp = days +
        std::chrono::hours { hourValue } +
        std::chrono::minutes { minuteValue } +
        std::chrono::seconds { secondValue } +
        std::chrono::milliseconds { millisecondValue };

    return std::chrono::time_point_cast<std::chrono::milliseconds>(timestamp);
}

std::string LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:
            return "TRACE";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Fatal:
            return "FATAL";
    }

    throw std::invalid_argument("invalid LogLevel");
}

std::optional<LogLevel> StringToLogLevel(std::string_view text) {
    if (text == "TRACE")
        return LogLevel::Trace;
    if (text == "DEBUG")
        return LogLevel::Debug;
    if (text == "INFO")
        return LogLevel::Info;
    if (text == "WARN")
        return LogLevel::Warn;
    if (text == "ERROR")
        return LogLevel::Error;
    if (text == "FATAL")
        return LogLevel::Fatal;

    return std::nullopt;
}

}
