#include "shell/CommandRegistry.h"

#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"

#include <iomanip>
#include <iostream>
#include <utility>

void CommandRegistry::add(std::unique_ptr<ICommand> command) {
    commands_[toLower(command->name())] = std::move(command);
}

const ICommand* CommandRegistry::find(const std::string& name) const {
    auto it = commands_.find(toLower(name));
    return it == commands_.end() ? nullptr : it->second.get();
}

std::vector<std::string> CommandRegistry::names() const {
    std::vector<std::string> result;
    for (const auto& item : commands_) {
        result.push_back(item.first);
    }
    return result;
}

void CommandRegistry::printHelp() const {
    ConsoleStyle::writeInfo("WinShellX commands:\n");
    for (const auto& item : commands_) {
        std::cout << "  " << std::left << std::setw(18) << item.second->usage()
                  << item.second->description() << "\n";
    }
}
