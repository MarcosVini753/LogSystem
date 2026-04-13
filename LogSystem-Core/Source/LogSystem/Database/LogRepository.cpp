#include <sqlite3.h>
#include "LogSystem/Database/LogRepository.h"
#include "LogSystem/Database/Database.h"
#include "LogSystem/Errors/DatabaseError.h"

namespace logsystem {

LogRepository::LogRepository(Database& db) : m_Db(db) {}
void LogRepository::InitSchema() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS logs ("
        "id INTEGER PRIMARY KEY,"
        "timestamp TEXT,"
        "level TEXT,"
        "message TEXT"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_Db.Handle(), sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::string msg = errMsg ? errMsg : "unknown sqlite error";
        sqlite3_free(errMsg);
        throw DatabaseError("sqlite3_exec failed: " + msg);
    }
}
void LogRepository::Insert(const Log&) {}

}