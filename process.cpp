#include "process.h"
#include "utils.h"

#include <cctype>
#include <cstdlib>
#include <sstream>

static int clampToUint16(int value)
{
    if (value < 0) return 0;
    if (value > 65535) return 65535;
    return value;
}

Process::Process(int pid, std::string name, RequirementFlags requirementFlags)
    : pid(pid), name(name), requirementFlags(requirementFlags), startTime(currentTimestamp())
{
    variables["x"] = 0;
}

void Process::addCommand(const Command& command)
{
    commandList.push_back(command);
}

int Process::valueOf(const std::string& operand) const
{
    if (!operand.empty() && std::isdigit(static_cast<unsigned char>(operand[0])))
        return std::stoi(operand);

    auto found = variables.find(operand);
    if (found == variables.end())
        return 0;
    return found->second;
}

void Process::addLog(int coreID, const std::string& message)
{
    logs.push_back("(" + currentTimestamp() + ") Core:" + std::to_string(coreID)
        + " \"" + message + "\"");
}

void Process::executeCurrentCommand(int coreID)
{
    if (isFinished()) return;

    cpuCoreID = coreID;
    currentState = RUNNING;

    const Command& command = commandList[commandCounter];

    if (command.type == PRINT)
    {
        if (command.second.empty())
            addLog(coreID, "Hello world from " + name + "!");
        else
            addLog(coreID, command.first + std::to_string(valueOf(command.second)));
    }
    else if (command.type == DECLARE)
    {
        variables[command.first] = static_cast<uint16_t>(clampToUint16(valueOf(command.second)));
    }
    else if (command.type == ADD)
    {
        int sum = valueOf(command.second) + valueOf(command.third);
        variables[command.first] = static_cast<uint16_t>(clampToUint16(sum));
    }
    else if (command.type == SUBTRACT)
    {
        int difference = valueOf(command.second) - valueOf(command.third);
        variables[command.first] = static_cast<uint16_t>(clampToUint16(difference));
    }
    else if (command.type == SLEEP)
    {
        sleepTicksLeft = valueOf(command.second);
        currentState = WAITING;
    }
    else if (command.type == FOR)
    {
        int iterations = valueOf(command.first);
        if (iterations <= 0)
        {
            nextInstruction = command.jumpIndex; // Skip loop
        }
        else
        {
            loopStack.push_back({ commandCounter, iterations });
            nextInstruction = commandCounter + 1;
        }
    }
    else if (command.type == ENDFOR)
    {
        if (!loopStack.empty())
        {
            auto& frame = loopStack.back();
            frame.iterationsLeft--;
            if (frame.iterationsLeft > 0)
            {
                nextInstruction = frame.forCommandIndex + 1; // Jump back to inside loop
            }
            else
            {
                loopStack.pop_back();
                nextInstruction = commandCounter + 1; // Exit loop
            }
        }
        else
        {
            nextInstruction = commandCounter + 1;
        }
    }
}

void Process::moveToNextLine()
{
    if (nextInstruction != -1)
    {
        commandCounter = nextInstruction;
        nextInstruction = -1;
    }
    else
    {
        commandCounter++;
    }

    if (isFinished())
        currentState = FINISHED;
}

bool Process::isFinished() const { return commandCounter >= static_cast<int>(commandList.size()); }
int Process::getRemainingTime() const { return getLinesOfCode() - commandCounter; }
int Process::getCommandCounter() const { return commandCounter; }
int Process::getLinesOfCode() const { return static_cast<int>(commandList.size()); }
int Process::getPID() const { return pid; }
int Process::getCPUCoreID() const { return cpuCoreID; }
Process::ProcessState Process::getState() const { return currentState; }
std::string Process::getName() const { return name; }
std::string Process::getStartTime() const { return startTime; }

std::string Process::getSmiReport() const
{
    std::ostringstream out;
    out << "Process name: " << name << "\n";
    out << "ID: " << pid << "\n";
    out << "Logs:\n";
    for (const std::string& line : logs)
        out << line << "\n";

    out << "\nCurrent instruction line: " << commandCounter << "\n";
    out << "Lines of code: " << getLinesOfCode() << "\n";

    if (isFinished())
        out << "\nFinished!\n";

    return out.str();
}

// Generates random commands, occasionally inserting up to 3 levels of nested FOR loops
void Process::test_generateRandomCommands(int limit)
{
    commandList.clear(); // Ensure we start fresh
    int executedInstructions = 0;
    std::vector<int> loopStack;

    while (executedInstructions < limit)
    {
        int choice = std::rand() % 100;

        // If we are close to the limit, or randomly chosen, generate flat instructions
        if (choice < 90 || commandList.size() + 2 >= limit)
        {
            int maxRand = 100;
            int subChoice = std::rand() % maxRand;

            if (subChoice < 30)
            {
                // PRINT: Empty strings trigger "Hello world from <name>!" in executeCurrentCommand()
                addCommand({ PRINT, "", "", "" });
            }
            else if (subChoice < 50)
            {
                addCommand({ DECLARE, "x", "0", "" });
            }
            else if (subChoice < 70)
            {
                addCommand({ ADD, "x", "x", "1" });
            }
            else if (subChoice < 85)
            {
                addCommand({ SUBTRACT, "x", "x", "1" });
            }
            else
            {
                addCommand({ SLEEP, "", "1", "" });
            }
            executedInstructions++;
        }
        else if (loopStack.size() < 3)
        {
            // Insert a FOR loop
            int iterations = 2 + std::rand() % 3; // 2 to 4 iterations
            int forIdx = commandList.size();
            addCommand({ FOR, std::to_string(iterations), "", "", -1 });
            loopStack.push_back(forIdx);
            executedInstructions++;
        }
        else if (!loopStack.empty())
        {
            // Close the FOR loop
            int forIdx = loopStack.back();
            loopStack.pop_back();
            int endIdx = commandList.size();
            addCommand({ ENDFOR, "", "", "", forIdx });
            commandList[forIdx].jumpIndex = endIdx;
            executedInstructions++;
        }
    }

    // Close any dangling loops safely
    while (!loopStack.empty())
    {
        int forIdx = loopStack.back();
        loopStack.pop_back();
        int endIdx = commandList.size();
        addCommand({ ENDFOR, "", "", "", forIdx });
        commandList[forIdx].jumpIndex = endIdx;
    }
}