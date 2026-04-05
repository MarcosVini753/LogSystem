#pragma once
#include <optional>
#include <string_view>
#include "LogSystem/Model/Log.h"

namespace logsystem {

class LogParser
{
public:
    // retorna nullopt se a linha for inválida
    std::optional<Log> ParseLine(std::string_view line) const;
};

}