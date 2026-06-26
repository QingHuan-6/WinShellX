#pragma once

#include "shell/ICommand.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

// Registry mapping command names to ICommand instances.
class CommandRegistry {
public:
    void add(std::unique_ptr<ICommand> command);

    const ICommand* find(const std::string& name) const;

    std::vector<std::string> names() const;

    void printHelp() const;

private:
    std::map<std::string, std::unique_ptr<ICommand>> commands_;
};
