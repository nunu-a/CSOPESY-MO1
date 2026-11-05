#pragma once

#include "config.h"
#include "process.h"

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class ResourceEmulator
{
public:
    explicit ResourceEmulator(const Config& settings);
    ~ResourceEmulator();

    void start();   // Starts the background CPU thread
    void stop();    // Stops it

    void startDummyProcesses();
    void stopDummyProcesses();

    bool createScreenProcess(const std::string& name);
    bool canAttach(const std::string& name);
    std::string getSmiReport(const std::string& name);
    std::string getUtilizationReport();

private:
    void tick();
    void wakeSleepingProcesses();
    void runCores();
    void fillFreeCores();
    void addProcess(const std::string& name);
    std::shared_ptr<Process> findProcess(const std::string& name) const;

    Config config;
    std::mutex mutex;                 // Protects everything below
    std::thread worker;
    std::atomic<bool> running{ false };
    bool generatingDummies = false;

    int cpuCycles = 0;
    int nextPid = 1;
    int dummyCount = 0;

    std::vector<std::shared_ptr<Process>> allProcesses;
    std::vector<std::shared_ptr<Process>> cores;   // nullptr means the core is free
    std::deque<std::shared_ptr<Process>> readyQueue;
};