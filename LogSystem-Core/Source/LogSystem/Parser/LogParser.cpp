#include "LogSystem/Parser/LogParser.h"

namespace logsystem {

std::optional<Log> LogParser::ParseLine(std::string_view line) const
{
    // TODO: implementar parser real
    // Temporário: se linha vazia, ignora; senão cria um Log “fake”
    if (line.empty())
        return std::nullopt;

    Log log;
    log.timestamp = "";
    log.level = "";
    log.message = std::string(line);
    return log;
}

}