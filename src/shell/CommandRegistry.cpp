#include "shell/CommandRegistry.h"

#include "utils/StringUtils.h"

#include <iomanip>
#include <iostream>
#include <utility>

void CommandRegistry::add(const std::string& name, CommandInfo command) {
    commands_[toLower(name)] = std::move(command);
}

const CommandInfo* CommandRegistry::find(const std::string& name) const {
    auto it = commands_.find(toLower(name));
    return it == commands_.end() ? nullptr : &it->second;
}

std::vector<std::string> CommandRegistry::names() const {
    std::vector<std::string> result;
    for (const auto& item : commands_) {
        result.push_back(item.first);
    }
    return result;
}

void CommandRegistry::printHelp() const {
    std::cout << "WinShellX commands:\n";
    for (const auto& item : commands_) {
        std::cout << "  " << std::left << std::setw(18) << item.second.usage
                  << item.second.description << "\n";
    }
}
