#pragma once
#include <stdexcept>
#include <string>

namespace logsystem {

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

}