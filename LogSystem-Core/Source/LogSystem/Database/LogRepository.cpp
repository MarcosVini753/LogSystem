#include <sqlite3.h>

#include <memory>
#include <string>

#include "LogSystem/Database/LogRepository.h"
#include "LogSystem/Database/Database.h"
#include "LogSystem/Errors/DatabaseError.h"
#include "LogSystem/Utils/TimeUtils.h"

namespace logsystem {
namespace {

void ThrowSqliteError(sqlite3* db, const std::string& operation) {
    const char* message = db ? sqlite3_errmsg(db) : "database is not open";
    throw DatabaseError(operation + " failed: " + message);
}

void BindText(sqlite3* db, sqlite3_stmt* statement, int index, const std::string& value) {
    const int rc = sqlite3_bind_text(
        statement,
        index,
        value.c_str(),
        static_cast<int>(value.size()),
        SQLITE_TRANSIENT
    );

    if (rc != SQLITE_OK)
        ThrowSqliteError(db, "sqlite3_bind_text");
}

} // namespace

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
    if (rc != SQLITE_OK){
        std::string msg = errMsg ? errMsg : "unknown sqlite error";
        sqlite3_free(errMsg);
        throw DatabaseError("sqlite3_exec failed: " + msg);
    }
}

void LogRepository::Insert(const Log& log) {
    sqlite3* db = m_Db.Handle();
    if (!db)
        ThrowSqliteError(db, "LogRepository::Insert");

    const char* sql =
        "INSERT INTO logs (timestamp, level, message) "
        "VALUES (?, ?, ?);";

    sqlite3_stmt* rawStatement = nullptr;
    const int prepareRc = sqlite3_prepare_v2(db, sql, -1, &rawStatement, nullptr);
    if (prepareRc != SQLITE_OK)
        ThrowSqliteError(db, "sqlite3_prepare_v2");

    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> statement(rawStatement, sqlite3_finalize);

    const std::string timestamp = FormatTimestampToIso8601(log.timestampUtc);
    const std::string level = LogLevelToString(log.level);

    BindText(db, statement.get(), 1, timestamp);
    BindText(db, statement.get(), 2, level);
    BindText(db, statement.get(), 3, log.message);

    const int stepRc = sqlite3_step(statement.get());
    if (stepRc != SQLITE_DONE)
        ThrowSqliteError(db, "sqlite3_step");
}

}
