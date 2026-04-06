#pragma once
#include <string>

namespace logsystem {

class Database {
public:
    void Open(const std::string& filePath);
    void Close();
};

}