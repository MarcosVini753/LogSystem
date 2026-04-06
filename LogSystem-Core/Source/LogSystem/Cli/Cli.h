#pragma once
#include "LogSystem/Service/LogService.h"

namespace logsystem {

class Cli {
public:
    explicit Cli(LogService& service);
    void Run();

private:
    LogService& m_Service;
};

} 