#include <sqlite3.h>

#include "LogSystem/Database/Database.h"
#include "LogSystem/Errors/DatabaseError.h"

namespace logsystem {

Database::Database() = default;

Database::~Database() {
    Close();
}

void Database::Open(const std::string& filePath) {
    if (m_Db)
        return;

    int rc = sqlite3_open(filePath.c_str(), &m_Db);
    if (rc != SQLITE_OK) {
        std::string msg = sqlite3_errmsg(m_Db);
        sqlite3_close(m_Db);
        m_Db = nullptr;
        throw DatabaseError("sqlite3_open failed: " + msg);
    }
}

void Database::Close() {
    if (!m_Db)
        return;

    sqlite3_close(m_Db);
    m_Db = nullptr;
}

}