#pragma once

#include <string>
#include <vector>

struct ShellContext {
    bool running = true;
    std::vector<std::string> history;
    std::string historyFilePath;
};
