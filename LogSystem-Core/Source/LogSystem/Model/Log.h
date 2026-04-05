#pragma once
#include <string>

namespace logsystem {

struct Log
{
    std::string timestamp; // por enquanto string; depois pode virar um tipo próprio
    std::string level;     // "ERROR", "INFO", etc.
    std::string message;
};

}