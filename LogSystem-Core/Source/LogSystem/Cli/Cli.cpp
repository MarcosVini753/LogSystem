#include "LogSystem/Cli/Cli.h"
#include <iostream>
#include <string>

namespace logsystem {

Cli::Cli(LogService& service) : m_Service(service) {}

void Cli::Run() {
    std::cout << "LogSystem CLI (digite 'sair' para encerrar)\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line == "sair" || line == "exit")
            break;

        // stub: por enquanto processa como se fosse uma linha de log
        auto status = m_Service.ProcessLine(line);
        if (status == ProcessLineStatus::Processed)
            std::cout << "OK\n";
        else
            std::cout << "Ignorado (linha inválida)\n";
    }
}

}