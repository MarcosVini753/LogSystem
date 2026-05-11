#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "LogSystem/Parser/LogParser.h"
#include "LogSystem/Database/Database.h"
#include "LogSystem/Database/LogRepository.h"

namespace logsystem {

enum class ProcessLineStatus {
    Processed,
    IgnoredInvalid
};

struct ProcessLinesResult {
    std::size_t processed = 0;
    std::size_t ignoredInvalid = 0;
};

class LogService {
public:
    LogService();
    void OpenDb(const std::string& filePath);

    ProcessLineStatus ProcessLine(std::string_view line);
    ProcessLinesResult ProcessLines(const std::vector<std::string>& lines);

private:
    Database m_Db;
    LogRepository m_Repo;
    LogParser m_Parser;
};

}
