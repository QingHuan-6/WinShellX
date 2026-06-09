#include "shell/Shell.h"

#include "commands/BuiltInCommands.h"
#include "shell/CommandParser.h"
#include "utils/PathUtils.h"
#include "utils/StringUtils.h"

#include <iostream>
#include <string>

Shell::Shell() : historyStore_(".winshellx_history") {
    context_.historyFilePath = historyStore_.filePath();
    context_.history = historyStore_.load();
    registerBuiltInCommands(registry_);
}

void Shell::run() {
    std::cout << "WinShellX started. Type help for commands.\n";
    std::cout << "History loaded from " << context_.historyFilePath
              << ". Tab completes history/commands/paths, F7 chooses recent history.\n";

    while (context_.running) {
        std::string prompt = getCurrentDirectoryText() + ">";

        std::string input = lineEditor_.readLine(prompt, context_.history, registry_.names());
        if (!std::cin && input.empty()) {
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

    historyStore_.save(context_.history);
}
