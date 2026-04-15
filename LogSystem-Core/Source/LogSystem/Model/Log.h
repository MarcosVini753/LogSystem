#pragma once
#include <chrono>
#include <cstdint>
#include <string>

namespace logsystem {

enum class LogLevel : uint8_t {
    Trace, Debug, Info, Warn, Error, Fatal
};

struct Log {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> timestampUtc;
    LogLevel level;
    std::string message;
};

}
