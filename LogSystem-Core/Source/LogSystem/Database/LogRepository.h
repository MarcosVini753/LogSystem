#pragma once
#include "LogSystem/Model/Log.h"

namespace logsystem {

class Database;

class LogRepository {
public:
    explicit LogRepository(Database& db);

    void InitSchema();
    void Insert(const Log& log);

private:
    Database& m_Db;
};

}