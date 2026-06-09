#pragma once

#include <map>
#include <string>
#include <vector>

struct ShellContext {
    bool running = true;
    std::vector<std::string> history;
    std::string historyFilePath;
    std::map<std::string, std::string> aliases;
    std::string aliasFilePath;
};
