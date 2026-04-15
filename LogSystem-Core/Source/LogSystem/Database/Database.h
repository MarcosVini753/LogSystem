#pragma once
#include <string>

struct sqlite3;

namespace logsystem {

class Database {
public:
    Database();
    ~Database();

    void Open(const std::string& filePath);
    void Close();

    sqlite3* Handle() const { return m_Db; }

private:
    sqlite3* m_Db = nullptr;
};

}