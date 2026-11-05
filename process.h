#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Process
{
public:
    struct RequirementFlags
    {
        bool requireFiles;
        int numFiles;
        bool requireMemory;
        int memoryRequired;
    };

    enum ProcessState { READY, RUNNING, WAITING, FINISHED };

    enum CommandType { PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR, ENDFOR };

    struct Command
    {
        CommandType type;
        std::string first;
        std::string second;
        std::string third;
        int jumpIndex = -1; // Used to jump to ENDFOR or FOR
    };

    Process(int pid, std::string name, RequirementFlags requirementFlags);

    void addCommand(const Command& command);
    void executeCurrentCommand(int coreID);
    void moveToNextLine();

    bool isFinished() const;
    int getRemainingTime() const;
    int getCommandCounter() const;
    int getLinesOfCode() const;
    int getPID() const;
    int getCPUCoreID() const;
    ProcessState getState() const;
    std::string getName() const;
    std::string getStartTime() const;

    std::string getSmiReport() const;
    void test_generateRandomCommands(int limit);

private:
    struct LoopFrame {
        int forCommandIndex;
        int iterationsLeft;
    };

    int pid;
    std::string name;
    std::vector<Command> commandList;

    int commandCounter = 0;
    int cpuCoreID = -1;
    int sleepTicksLeft = 0;
    int sliceTicksUsed = 0;
    RequirementFlags requirementFlags;
    ProcessState currentState = READY;
    std::string startTime;
    std::vector<std::string> logs;
    std::map<std::string, uint16_t> variables;

    // New variables for loops and delays
    int delayTicksLeft = 0;
    int nextInstruction = -1;
    std::vector<LoopFrame> loopStack;

    int valueOf(const std::string& operand) const;
    void addLog(int coreID, const std::string& message);

    friend class ResourceEmulator;
};