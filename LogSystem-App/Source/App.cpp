#include "Core.h"

int main() {

    logsystem::LogService service;
    service.OpenDb("logsystem.db"); // por enquanto stub

    logsystem::Cli cli(service);
    cli.Run();

	// Core::PrintHelloWorld();
    return 0;
}