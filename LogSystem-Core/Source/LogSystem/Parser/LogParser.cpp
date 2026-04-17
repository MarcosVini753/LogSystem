#include "LogSystem/Parser/LogParser.h"

#include <chrono>

namespace logsystem {
namespace {

bool IsDigit(char ch){
    return ch >= '0' && ch <= '9';
}

bool ParseFixedInt(std::string_view text, int& value){
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

std::optional<LogLevel> ParseLogLevel(std::string_view levelText){
    if (levelText == "TRACE")
        return LogLevel::Trace;
    if (levelText == "DEBUG")
        return LogLevel::Debug;
    if (levelText == "INFO")
        return LogLevel::Info;
    if (levelText == "WARN")
        return LogLevel::Warn;
    if (levelText == "ERROR")
        return LogLevel::Error;
    if (levelText == "FATAL")
        return LogLevel::Fatal;

    return std::nullopt;
}

std::optional<TimestampUtc> ParseTimestampUtc(std::string_view timestampText){
    // Canonical format: YYYY-MM-DDTHH:MM:SS.mmmZ
    if (timestampText.size() != 24)
        return std::nullopt;

    if (timestampText[4] != '-' || timestampText[7] != '-' || timestampText[10] != 'T' ||
        timestampText[13] != ':' || timestampText[16] != ':' || timestampText[19] != '.' ||
        timestampText[23] != 'Z') {
        return std::nullopt;
    }

    int yearValue = 0;
    int monthValue = 0;
    int dayValue = 0;
    int hourValue = 0;
    int minuteValue = 0;
    int secondValue = 0;
    int millisecondValue = 0;

    if (!ParseFixedInt(timestampText.substr(0, 4), yearValue) ||
        !ParseFixedInt(timestampText.substr(5, 2), monthValue) ||
        !ParseFixedInt(timestampText.substr(8, 2), dayValue) ||
        !ParseFixedInt(timestampText.substr(11, 2), hourValue) ||
        !ParseFixedInt(timestampText.substr(14, 2), minuteValue) ||
        !ParseFixedInt(timestampText.substr(17, 2), secondValue) ||
        !ParseFixedInt(timestampText.substr(20, 3), millisecondValue)) {
        return std::nullopt;
    }

    const std::chrono::year_month_day ymd {
        std::chrono::year { yearValue },
        std::chrono::month { static_cast<unsigned>(monthValue) },
        std::chrono::day { static_cast<unsigned>(dayValue) }
    };

    if (!ymd.ok())
        return std::nullopt;

    if (hourValue < 0 || hourValue > 23 ||
        minuteValue < 0 || minuteValue > 59 ||
        secondValue < 0 || secondValue > 59) {
        return std::nullopt;
    }

    const auto days = std::chrono::sys_days { ymd };
    const auto timestamp = days +
        std::chrono::hours { hourValue } +
        std::chrono::minutes { minuteValue } +
        std::chrono::seconds { secondValue } +
        std::chrono::milliseconds { millisecondValue };

    return std::chrono::time_point_cast<std::chrono::milliseconds>(timestamp);
}

} // namespace

std::optional<Log> LogParser::ParseLine(std::string_view line) const {
    const auto firstSpace = line.find(' ');
    if (firstSpace == std::string_view::npos)
        return std::nullopt;

    const auto secondSpace = line.find(' ', firstSpace + 1);
    if (secondSpace == std::string_view::npos)
        return std::nullopt;

    const std::string_view timestampText = line.substr(0, firstSpace);
    const std::string_view levelText = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    const std::string_view messageText = line.substr(secondSpace + 1);

    if (timestampText.empty() || levelText.empty() || messageText.empty())
        return std::nullopt;

    const auto timestampUtc = ParseTimestampUtc(timestampText);
    if (!timestampUtc)
        return std::nullopt;

    const auto level = ParseLogLevel(levelText);
    if (!level)
        return std::nullopt;

    return Log {
        .timestampUtc = *timestampUtc,
        .level = *level,
        .message = std::string(messageText)
    };
}

}