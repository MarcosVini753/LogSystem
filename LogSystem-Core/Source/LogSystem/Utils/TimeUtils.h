#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "LogSystem/Model/Log.h"

namespace logsystem {

std::string FormatTimestampToIso8601(TimestampUtc timestamp);
std::optional<TimestampUtc> ParseTimestampFromIso8601(std::string_view text);

std::string LogLevelToString(LogLevel level);
std::optional<LogLevel> StringToLogLevel(std::string_view text);

}
