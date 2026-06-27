#include "commands/WhereCommand.h"

#include "shell/CommandResolver.h"
#include "utils/ConsoleStyle.h"

#include <iostream>

std::string WhereCommand::name() const { return "where"; }
std::string WhereCommand::usage() const { return "where <command>"; }
std::string WhereCommand::description() const { return "Show full path of an executable searched via PATH/PATHEXT"; }

void WhereCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.empty()) {
        ConsoleStyle::writeError("where: missing command name. Usage: " + usage() + "\n");
        return;
    }

    for (const std::string& name : args) {
        std::vector<std::string> paths = resolveAllExecutablePaths(name);
        if (paths.empty()) {
            ConsoleStyle::writeError("INFO: Could not find " + name + " along PATH.\n");
            continue;
        }
        for (const std::string& path : paths) {
            std::cout << path << "\n";
        }
    }
}
