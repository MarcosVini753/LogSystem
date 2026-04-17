#pragma once
#include <chrono>
#include <cstdint>
#include <string>

namespace logsystem {

using TimestampUtc = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

enum class LogLevel : uint8_t {
    Trace, Debug, Info, Warn, Error, Fatal
};

struct Log {
    TimestampUtc timestampUtc;
    LogLevel level;
    std::string message;
};

}
