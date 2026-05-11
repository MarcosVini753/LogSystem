#include "LogSystem/Service/LogService.h"

namespace logsystem {

LogService::LogService() : m_Db(), m_Repo(m_Db), m_Parser() {}

void LogService::OpenDb(const std::string& filePath) {
    m_Db.Open(filePath);
    m_Repo.InitSchema();
}

ProcessLineStatus LogService::ProcessLine(std::string_view line) {
    auto log = m_Parser.ParseLine(line);
    if (!log)
        return ProcessLineStatus::IgnoredInvalid;

    m_Repo.Insert(*log);
    return ProcessLineStatus::Processed;
}

ProcessLinesResult LogService::ProcessLines(const std::vector<std::string>& lines) {
    ProcessLinesResult result;
    std::vector<Log> validLogs;
    validLogs.reserve(lines.size());

    for (const std::string& line : lines) {
        auto log = m_Parser.ParseLine(line);
        if (!log) {
            ++result.ignoredInvalid;
            continue;
        }

        validLogs.push_back(*log);
    }

    m_Repo.InsertMany(validLogs);
    result.processed = validLogs.size();
    return result;
}

} 
