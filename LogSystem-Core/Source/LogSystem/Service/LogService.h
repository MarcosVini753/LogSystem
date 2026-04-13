#pragma once
#include <string>
#include <string_view>
#include "LogSystem/Parser/LogParser.h"
#include "LogSystem/Database/Database.h"
#include "LogSystem/Database/LogRepository.h"

namespace logsystem {

enum class ProcessLineStatus {
    Processed,
    IgnoredInvalid
};

class LogService {
public:
    LogService();
    void OpenDb(const std::string& filePath);

    ProcessLineStatus ProcessLine(std::string_view line);

private:
    Database m_Db;
    LogRepository m_Repo;
    LogParser m_Parser;
};

}