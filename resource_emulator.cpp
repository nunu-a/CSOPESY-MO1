#include "resource_emulator.h"
#include "utils.h"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <sstream>

// One CPU cycle takes this long
static const int CYCLE_MS = 100;

ResourceEmulator::ResourceEmulator(const Config& settings)
    : config(settings),
    cores(settings.numCpu)
{
}

ResourceEmulator::~ResourceEmulator()
{
    stop();
}

void ResourceEmulator::start()
{
    running = true;
    worker = std::thread([this]() {
        while (running)
        {
            tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(CYCLE_MS));
        }
        });
}

void ResourceEmulator::stop()
{
    running = false;
    if (worker.joinable())
        worker.join();
}

void ResourceEmulator::startDummyProcesses()
{
    std::lock_guard<std::mutex> lock(mutex);
    generatingDummies = true;
}

void ResourceEmulator::stopDummyProcesses()
{
    std::lock_guard<std::mutex> lock(mutex);
    generatingDummies = false;
}

// One CPU cycle. Called by the worker thread.
void ResourceEmulator::tick()
{
    std::lock_guard<std::mutex> lock(mutex);
    cpuCycles++;

    if (generatingDummies && cpuCycles % config.batchProcessFreq == 0)
    {
        dummyCount++;
        addProcess((dummyCount < 10 ? "p0" : "p") + std::to_string(dummyCount));
    }

    wakeSleepingProcesses();
    runCores();
    fillFreeCores();
}

void ResourceEmulator::addProcess(const std::string& name)
{
    Process::RequirementFlags flags = { false, 0, false, 0 };
    int lines = config.minIns + std::rand() % (config.maxIns - config.minIns + 1);

    auto process = std::make_shared<Process>(nextPid++, name, flags);
    process->test_generateRandomCommands(lines);

    allProcesses.push_back(process);
    readyQueue.push_back(process);
}

// Processes that are sleeping count down. When they reach zero, they go back to the queue.
void ResourceEmulator::wakeSleepingProcesses()
{
    for (const auto& process : allProcesses)
    {
        if (process->getState() != Process::WAITING)
            continue;

        process->sleepTicksLeft--;
        if (process->sleepTicksLeft <= 0)
        {
            process->currentState = Process::READY;
            readyQueue.push_back(process);
        }
    }
}

// Each busy core runs one instruction
void ResourceEmulator::runCores()
{
    for (int core = 0; core < config.numCpu; core++)
    {
        auto process = cores[core];
        if (!process)
            continue;

        // --- NEW DELAY LOGIC ---
        if (process->delayTicksLeft > 0)
        {
            process->delayTicksLeft--;
        }
        else
        {
            process->executeCurrentCommand(core);
            process->moveToNextLine();
            process->delayTicksLeft = config.delaysPerExec;
        }
        // -----------------------

        process->sliceTicksUsed++;

        bool sliceUsedUp = config.scheduler == "rr" &&
            process->sliceTicksUsed >= config.quantumCycles;

        if (process->isFinished())
        {
            cores[core] = nullptr;
        }
        else if (process->getState() == Process::WAITING)
        {
            cores[core] = nullptr;
        }
        else if (sliceUsedUp)
        {
            cores[core] = nullptr;
            process->currentState = Process::READY;
            readyQueue.push_back(process);
        }
    }
}
// Each free core takes the next process from the queue
void ResourceEmulator::fillFreeCores()
{
    for (int core = 0; core < config.numCpu && !readyQueue.empty(); core++)
    {
        if (cores[core])
            continue;

        auto process = readyQueue.front();
        readyQueue.pop_front();

        process->currentState = Process::RUNNING;
        process->sliceTicksUsed = 0;
        cores[core] = process;
    }
}

std::shared_ptr<Process> ResourceEmulator::findProcess(const std::string& name) const
{
    for (const auto& process : allProcesses)
    {
        if (process->getName() == name)
            return process;
    }
    return nullptr;
}

bool ResourceEmulator::createScreenProcess(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (findProcess(name))
        return false;

    addProcess(name);
    return true;
}

bool ResourceEmulator::canAttach(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto process = findProcess(name);
    return process && !process->isFinished();
}

std::string ResourceEmulator::getSmiReport(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto process = findProcess(name);
    if (!process)
        return "Process not found.\n";
    return process->getSmiReport();
}

std::string ResourceEmulator::getUtilizationReport()
{
    std::lock_guard<std::mutex> lock(mutex);

    int used = 0;
    for (const auto& process : cores)
    {
        if (process)
            used++;
    }

    std::ostringstream out;
    out << "CPU utilization: " << used * 100 / config.numCpu << "%\n";
    out << "Cores used: " << used << "\n";
    out << "Cores available: " << config.numCpu - used << "\n\n";
    out << "---------------------------------------\n";

    out << "Running processes:\n";
    for (int core = 0; core < config.numCpu; core++)
    {
        const auto& p = cores[core];
        if (!p)
            continue;

        out << std::left << std::setw(12) << p->getName()
            << "(" << p->getStartTime() << ")  Core: " << core << "   "
            << p->getCommandCounter() << " / " << p->getLinesOfCode() << "\n";
    }

    out << "\nFinished processes:\n";
    for (const auto& p : allProcesses)
    {
        if (!p->isFinished())
            continue;

        out << std::left << std::setw(12) << p->getName()
            << "(" << p->getStartTime() << ")  Finished  "
            << p->getLinesOfCode() << " / " << p->getLinesOfCode() << "\n";
    }

    out << "---------------------------------------\n";
    return out.str();
}