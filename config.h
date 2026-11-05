#pragma once
#include <algorithm>
#include <fstream>
#include <string>

// Settings from config.txt. The defaults match the sample file.
struct Config
{
    int numCpu = 4;
    std::string scheduler = "rr";
    int quantumCycles = 5;
    int batchProcessFreq = 1;
    int minIns = 1000;
    int maxIns = 2000;
    int delaysPerExec = 0;
};

// Reads "name value" pairs. Returns false if the file is missing.
inline bool loadConfig(const std::string& path, Config& config)
{
    std::ifstream file(path);
    if (!file)
        return false;

    std::string key;
    while (file >> key)
    {
        if (key == "num-cpu")                 file >> config.numCpu;
        else if (key == "scheduler")          file >> config.scheduler;
        else if (key == "quantum-cycles")     file >> config.quantumCycles;
        else if (key == "batch-process-freq") file >> config.batchProcessFreq;
        else if (key == "min-ins")            file >> config.minIns;
        else if (key == "max-ins")            file >> config.maxIns;
        else if (key == "delay-per-exec")     file >> config.delaysPerExec;
    }

    // The sample file puts the scheduler name in quotes: "rr"
    config.scheduler.erase(
        std::remove(config.scheduler.begin(), config.scheduler.end(), '"'),
        config.scheduler.end());

    return true;
}