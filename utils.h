#pragma once
#pragma once
#include <ctime>
#include <string>

// Current local time, for example "01/18/2024 09:15:22AM"
inline std::string currentTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S%p", &local);
    return buffer;
}