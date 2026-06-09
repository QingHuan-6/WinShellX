#include "shell/Shell.h"

#include "commands/BuiltInCommands.h"
#include "shell/CommandParser.h"
#include "utils/PathUtils.h"
#include "utils/StringUtils.h"

#include <iostream>
#include <string>

Shell::Shell() {
    registerBuiltInCommands(registry_);
}

void Shell::run() {
    std::cout << "WinShellX started. Type help for commands.\n";

    while (context_.running) {
        std::cout << getCurrentDirectoryText() << ">";

        std::string input;
        if (!std::getline(std::cin, input)) {
            break;
        }

        input = trim(input);
        if (input.empty()) {
            continue;
        }

        context_.history.push_back(input);
        ParsedCommand command = parseCommand(input);
        const CommandInfo* info = registry_.find(command.name);

        if (!info) {
            std::cerr << "Unknown command: " << command.name << "\n";
            continue;
        }

        info->handler(context_, command.args);
    }
}
