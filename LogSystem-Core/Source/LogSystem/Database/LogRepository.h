#pragma once
#include <vector>

#include "LogSystem/Model/Log.h"

namespace logsystem {

class Database;

class LogRepository {
public:
    explicit LogRepository(Database& db);

    void InitSchema();
    void Insert(const Log& log);
    void InsertMany(const std::vector<Log>& logs);

private:
    Database& m_Db;
};

}
