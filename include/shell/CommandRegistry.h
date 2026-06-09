#pragma once

#include "shell/CommandInfo.h"

#include <map>
#include <string>

class CommandRegistry {
public:
    void add(const std::string& name, CommandInfo command);
    const CommandInfo* find(const std::string& name) const;
    void printHelp() const;

private:
    std::map<std::string, CommandInfo> commands_;
};
