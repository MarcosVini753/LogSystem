#include <sqlite3.h>

#include <memory>
#include <string>
#include <vector>

#include "LogSystem/Database/LogRepository.h"
#include "LogSystem/Database/Database.h"
#include "LogSystem/Errors/DatabaseError.h"
#include "LogSystem/Utils/TimeUtils.h"

namespace logsystem {
namespace {

constexpr int CurrentSchemaVersion = 1;

using StatementPtr = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

void ThrowSqliteError(sqlite3* db, const std::string& context) {
    const char* message = db ? sqlite3_errmsg(db) : "database is not open";
    throw DatabaseError(context + " failed: " + message);
}

void ExecuteSql(sqlite3* db, const char* sql, const std::string& context) {
    char* errMsg = nullptr;
    const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        const std::string message = errMsg ? errMsg : "unknown sqlite error";
        sqlite3_free(errMsg);
        throw DatabaseError(context + " failed: " + message);
    }
}

void RollbackQuietly(sqlite3* db) {
    char* errMsg = nullptr;
    sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg);
    sqlite3_free(errMsg);
}

StatementPtr PrepareStatement(sqlite3* db, const char* sql, const std::string& context) {
    sqlite3_stmt* rawStatement = nullptr;
    const int rc = sqlite3_prepare_v2(db, sql, -1, &rawStatement, nullptr);
    if (rc != SQLITE_OK)
        ThrowSqliteError(db, context);

    return StatementPtr(rawStatement, sqlite3_finalize);
}

int QueryInt(sqlite3* db, const char* sql, const std::string& context) {
    auto statement = PrepareStatement(db, sql, context + " prepare");

    const int stepRc = sqlite3_step(statement.get());
    if (stepRc != SQLITE_ROW)
        ThrowSqliteError(db, context + " read");

    return sqlite3_column_int(statement.get(), 0);
}

int GetUserVersion(sqlite3* db) {
    return QueryInt(db, "PRAGMA user_version;", "LogRepository::InitSchema get user_version");
}

void SetUserVersion(sqlite3* db, int version) {
    const std::string sql = "PRAGMA user_version = " + std::to_string(version) + ";";
    ExecuteSql(db, sql.c_str(), "LogRepository::InitSchema set user_version");
}

bool LogsTableExists(sqlite3* db) {
    return QueryInt(
        db,
        "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = 'logs';",
        "LogRepository::InitSchema check logs table"
    ) > 0;
}

const char* CreateLogsTableSql(const char* tableName) {
    if (std::string(tableName) == "logs_new") {
        return
            "CREATE TABLE logs_new ("
            "id INTEGER PRIMARY KEY,"
            "timestamp TEXT NOT NULL,"
            "level TEXT NOT NULL CHECK(level IN ('TRACE', 'DEBUG', 'INFO', 'WARN', 'ERROR', 'FATAL')),"
            "message TEXT NOT NULL"
            ");";
    }

    return
        "CREATE TABLE IF NOT EXISTS logs ("
        "id INTEGER PRIMARY KEY,"
        "timestamp TEXT NOT NULL,"
        "level TEXT NOT NULL CHECK(level IN ('TRACE', 'DEBUG', 'INFO', 'WARN', 'ERROR', 'FATAL')),"
        "message TEXT NOT NULL"
        ");";
}

void CreateCurrentLogsTable(sqlite3* db) {
    ExecuteSql(db, CreateLogsTableSql("logs"), "LogRepository::InitSchema create logs table");
    SetUserVersion(db, CurrentSchemaVersion);
}

void ValidateLegacyLogsCanMigrate(sqlite3* db) {
    const int invalidRows = QueryInt(
        db,
        "SELECT COUNT(*) FROM logs "
        "WHERE timestamp IS NULL "
        "OR message IS NULL "
        "OR level IS NULL "
        "OR level NOT IN ('TRACE', 'DEBUG', 'INFO', 'WARN', 'ERROR', 'FATAL');",
        "LogRepository::MigrateLegacySchemaToV1 validate legacy rows"
    );

    if (invalidRows > 0) {
        throw DatabaseError(
            "LogRepository::MigrateLegacySchemaToV1 aborted: legacy logs table contains " +
            std::to_string(invalidRows) +
            " invalid row(s). Fix or remove invalid rows before migration."
        );
    }
}

void MigrateLegacySchemaToV1(sqlite3* db) {
    ValidateLegacyLogsCanMigrate(db);

    ExecuteSql(db, "BEGIN IMMEDIATE TRANSACTION;", "LogRepository::MigrateLegacySchemaToV1 begin transaction");

    try {
        ExecuteSql(db, CreateLogsTableSql("logs_new"), "LogRepository::MigrateLegacySchemaToV1 create logs_new");
        ExecuteSql(
            db,
            "INSERT INTO logs_new (id, timestamp, level, message) "
            "SELECT id, timestamp, level, message FROM logs;",
            "LogRepository::MigrateLegacySchemaToV1 copy logs"
        );
        ExecuteSql(db, "DROP TABLE logs;", "LogRepository::MigrateLegacySchemaToV1 drop legacy logs");
        ExecuteSql(db, "ALTER TABLE logs_new RENAME TO logs;", "LogRepository::MigrateLegacySchemaToV1 rename logs_new");
        SetUserVersion(db, CurrentSchemaVersion);
        ExecuteSql(db, "COMMIT;", "LogRepository::MigrateLegacySchemaToV1 commit");
    } catch (...) {
        RollbackQuietly(db);
        throw;
    }
}

void BindText(
    sqlite3* db,
    sqlite3_stmt* statement,
    int index,
    const std::string& context,
    const std::string& fieldName,
    const std::string& value
) {
    const int rc = sqlite3_bind_text(
        statement,
        index,
        value.c_str(),
        static_cast<int>(value.size()),
        SQLITE_TRANSIENT
    );

    if (rc != SQLITE_OK)
        ThrowSqliteError(db, context + " bind " + fieldName);
}

StatementPtr PrepareInsertStatement(sqlite3* db, const std::string& context) {
    return PrepareStatement(
        db,
        "INSERT INTO logs (timestamp, level, message) VALUES (?, ?, ?);",
        context + " prepare"
    );
}

void ExecuteInsertStatement(sqlite3* db, sqlite3_stmt* statement, const Log& log, const std::string& context) {
    const std::string timestamp = FormatTimestampToIso8601(log.timestampUtc);
    const std::string level = LogLevelToString(log.level);

    BindText(db, statement, 1, context, "timestamp", timestamp);
    BindText(db, statement, 2, context, "level", level);
    BindText(db, statement, 3, context, "message", log.message);

    const int stepRc = sqlite3_step(statement);
    if (stepRc != SQLITE_DONE)
        ThrowSqliteError(db, context + " execute");

    const int resetRc = sqlite3_reset(statement);
    if (resetRc != SQLITE_OK)
        ThrowSqliteError(db, context + " reset");

    const int clearRc = sqlite3_clear_bindings(statement);
    if (clearRc != SQLITE_OK)
        ThrowSqliteError(db, context + " clear bindings");
}

} // namespace

LogRepository::LogRepository(Database& db) : m_Db(db) {}

void LogRepository::InitSchema() {
    sqlite3* db = m_Db.Handle();
    if (!db)
        ThrowSqliteError(db, "LogRepository::InitSchema");

    const int userVersion = GetUserVersion(db);
    if (userVersion > CurrentSchemaVersion) {
        throw DatabaseError(
            "LogRepository::InitSchema failed: database schema version " +
            std::to_string(userVersion) +
            " is newer than supported version " +
            std::to_string(CurrentSchemaVersion)
        );
    }

    if (!LogsTableExists(db)) {
        CreateCurrentLogsTable(db);
        return;
    }

    if (userVersion == 0) {
        MigrateLegacySchemaToV1(db);
        return;
    }
}

void LogRepository::Insert(const Log& log) {
    sqlite3* db = m_Db.Handle();
    if (!db)
        ThrowSqliteError(db, "LogRepository::Insert");

    auto statement = PrepareInsertStatement(db, "LogRepository::Insert");
    ExecuteInsertStatement(db, statement.get(), log, "LogRepository::Insert");
}

void LogRepository::InsertMany(const std::vector<Log>& logs) {
    if (logs.empty())
        return;

    sqlite3* db = m_Db.Handle();
    if (!db)
        ThrowSqliteError(db, "LogRepository::InsertMany");

    ExecuteSql(db, "BEGIN IMMEDIATE TRANSACTION;", "LogRepository::InsertMany begin transaction");

    try {
        auto statement = PrepareInsertStatement(db, "LogRepository::InsertMany");
        for (const Log& log : logs)
            ExecuteInsertStatement(db, statement.get(), log, "LogRepository::InsertMany");

        ExecuteSql(db, "COMMIT;", "LogRepository::InsertMany commit");
    } catch (...) {
        RollbackQuietly(db);
        throw;
    }
}

}
