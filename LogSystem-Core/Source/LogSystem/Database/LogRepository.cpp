#include "LogSystem/Database/LogRepository.h"
#include "LogSystem/Database/Database.h"

namespace logsystem {

LogRepository::LogRepository(Database& db) : m_Db(db) {}
void LogRepository::InitSchema() {}
void LogRepository::Insert(const Log&) {}

}