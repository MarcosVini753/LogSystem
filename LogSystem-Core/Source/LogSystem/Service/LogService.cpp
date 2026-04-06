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

}