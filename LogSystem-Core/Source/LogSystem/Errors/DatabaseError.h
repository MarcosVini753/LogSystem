#pragma once
#include <stdexcept>
#include <string>

namespace logsystem {

class DatabaseError : public std::runtime_error {
public:
    explicit DatabaseError(const std::string& msg) : std::runtime_error(msg) {}
};

}