#include "config.h"
#include "resource_emulator.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

static void printBanner()
{
    std::cout << "Welcome to CSOPESY Emulator!\n";
    std::cout << "Developers: Azzam Alonto\n\n";
}

static void clearConsole()
{
    std::system("cls");
}

// Stays in the process screen until the user types "exit"
static void runScreenSession(ResourceEmulator& emulator, const std::string& name)
{
    clearConsole();

    std::string line;
    while (true)
    {
        std::cout << "root:\\> ";
        if (!std::getline(std::cin, line))
            return;

        std::istringstream input(line);
        std::string command;
        input >> command;

        if (command == "process-smi")
        {
            std::cout << emulator.getSmiReport(name);
        }
        else if (command == "exit")
        {
            clearConsole();
            return;
        }
        else if (!command.empty())
        {
            std::cout << "Unknown command.\n";
        }
    }
}

int main()
{
    printBanner();

    Config config;
    std::unique_ptr<ResourceEmulator> emulator;   // Created by "initialize"
    std::string line;

    while (true)
    {
        std::cout << "root:\\> ";
        if (!std::getline(std::cin, line))
            break;

        std::istringstream input(line);
        std::string command, option, argument;
        input >> command >> option >> argument;

        if (command == "exit")
            break;

        if (command == "initialize")
        {
            if (!loadConfig("config.txt", config))
            {
                std::cout << "Could not open config.txt\n";
                continue;
            }
            emulator = std::make_unique<ResourceEmulator>(config);
            emulator->start();
            std::cout << "Emulator initialized.\n";
            continue;
        }

        // Every other command needs initialize first
        if (!emulator)
        {
            std::cout << "Type 'initialize' first.\n";
            continue;
        }

        if (command == "screen")
        {
            if (option == "-s" && !argument.empty())
            {
                if (emulator->createScreenProcess(argument))
                    runScreenSession(*emulator, argument);
                else
                    std::cout << "Process name already exists.\n";
            }
            else if (option == "-r" && !argument.empty())
            {
                if (emulator->canAttach(argument))
                    runScreenSession(*emulator, argument);
                else
                    std::cout << "Process " << argument << " not found.\n";
            }
            else if (option == "-ls")
            {
                std::cout << emulator->getUtilizationReport();
            }
            else
            {
                std::cout << "Usage: screen -s <name> | screen -r <name> | screen -ls\n";
            }
        }
        else if (command == "scheduler-start")
        {
            emulator->startDummyProcesses();
            std::cout << "Dummy process generation started.\n";
        }
        else if (command == "scheduler-stop")
        {
            emulator->stopDummyProcesses();
            std::cout << "Dummy process generation stopped.\n";
        }
        else if (command == "report-util")
        {
            std::ofstream file("csopesy-log.txt");
            file << emulator->getUtilizationReport();
            std::cout << "Report generated at csopesy-log.txt!\n";
        }
        else if (!command.empty())
        {
            std::cout << "Unknown command.\n";
        }
    }

    if (emulator)
        emulator->stop();

    return 0;
}