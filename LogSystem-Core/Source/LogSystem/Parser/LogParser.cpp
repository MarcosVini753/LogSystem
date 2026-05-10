#include "LogSystem/Parser/LogParser.h"
#include "LogSystem/Utils/TimeUtils.h"

namespace logsystem {

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

    const auto timestampUtc = ParseTimestampFromIso8601(timestampText);
    if (!timestampUtc)
        return std::nullopt;

    const auto level = StringToLogLevel(levelText);
    if (!level)
        return std::nullopt;

    return Log {
        .timestampUtc = *timestampUtc,
        .level = *level,
        .message = std::string(messageText)
    };
}

}
