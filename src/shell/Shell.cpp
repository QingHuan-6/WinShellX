#include "shell/Shell.h"

#include "commands/BuiltInCommands.h"
#include "shell/CommandParser.h"
#include "utils/ConsoleStyle.h"
#include "utils/PathUtils.h"
#include "utils/StringUtils.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {
struct ShellInputPlan {
    std::string leftCommand;
    std::string rightCommand;
    std::string outputFile;
    bool background = false;
};

size_t findUnquoted(const std::string& input, char target) {
    bool inQuotes = false;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        if (!inQuotes && input[i] == target) {
            return i;
        }
    }
    return std::string::npos;
}

std::string unquote(std::string value) {
    value = trim(value);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

ShellInputPlan parseShellInput(const std::string& input) {
    ShellInputPlan plan;
    std::string commandPart = input;

    size_t backgroundPos = findUnquoted(commandPart, '&');
    if (backgroundPos != std::string::npos && trim(commandPart.substr(backgroundPos + 1)).empty()) {
        plan.background = true;
        commandPart = trim(commandPart.substr(0, backgroundPos));
    }

    size_t redirectPos = findUnquoted(commandPart, '>');
    if (redirectPos != std::string::npos) {
        plan.outputFile = unquote(commandPart.substr(redirectPos + 1));
        commandPart = trim(commandPart.substr(0, redirectPos));
    }

    size_t pipePos = findUnquoted(commandPart, '|');
    if (pipePos != std::string::npos) {
        plan.leftCommand = trim(commandPart.substr(0, pipePos));
        plan.rightCommand = trim(commandPart.substr(pipePos + 1));
    } else {
        plan.leftCommand = trim(commandPart);
    }

    return plan;
}

std::string firstToken(const std::string& input, size_t& endPos) {
    size_t index = 0;
    while (index < input.size() && std::isspace(static_cast<unsigned char>(input[index]))) {
        ++index;
    }

    if (index >= input.size()) {
        endPos = input.size();
        return "";
    }

    if (input[index] == '"') {
        size_t start = ++index;
        while (index < input.size() && input[index] != '"') {
            ++index;
        }
        endPos = index < input.size() ? index + 1 : index;
        return input.substr(start, index - start);
    }

    size_t start = index;
    while (index < input.size() && !std::isspace(static_cast<unsigned char>(input[index]))) {
        ++index;
    }
    endPos = index;
    return input.substr(start, index - start);
}
}

Shell::Shell() : aliasStore_(".winshellx_aliases"), historyStore_(".winshellx_history") {
    context_.aliasFilePath = aliasStore_.filePath();
    context_.aliases = aliasStore_.load();
    context_.historyFilePath = historyStore_.filePath();
    context_.history = historyStore_.load();
    registerBuiltInCommands(registry_);
}

void Shell::run() {
    ConsoleStyle::writeSuccess("WinShellX started. Type help for commands.\n");
    ConsoleStyle::writeInfo("History loaded from " + context_.historyFilePath +
                            ". Aliases loaded from " + context_.aliasFilePath + ".\n");
    ConsoleStyle::writeInfo("Tab completes history/commands/paths, Up/Down browses history, F7 chooses recent history.\n");

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
        executeInput(expandAlias(input));
    }

    aliasStore_.save(context_.aliases);
    historyStore_.save(context_.history);
}

bool Shell::executeInput(const std::string& input) {
    ShellInputPlan plan = parseShellInput(input);
    if (plan.leftCommand.empty()) {
        return false;
    }

    if (plan.background && (!plan.rightCommand.empty() || !plan.outputFile.empty())) {
        ConsoleStyle::writeError("Background mode does not support pipes or redirection yet.\n");
        return false;
    }

    if (!plan.rightCommand.empty()) {
        std::string pipeText;
        if (!executeSingle(plan.leftCommand, nullptr, &pipeText, "")) {
            return false;
        }
        return executeSingle(plan.rightCommand, &pipeText, nullptr, plan.outputFile);
    }

    return executeSingle(plan.leftCommand, nullptr, nullptr, plan.outputFile, plan.background);
}

bool Shell::executeSingle(
    const std::string& input,
    const std::string* stdinText,
    std::string* capturedOutput,
    const std::string& outputFilePath,
    bool background) {
    ParsedCommand command = parseCommand(input);
    const CommandInfo* info = registry_.find(command.name);

    if (!info) {
        ExternalRunOptions options;
        options.stdinText = stdinText;
        options.capturedOutput = capturedOutput;
        options.outputFilePath = outputFilePath;
        options.waitForExit = !background;
        return externalCommandRunner_.run(input, options);
    }

    if (background) {
        ConsoleStyle::writeError("Background mode is only supported for external commands.\n");
        return false;
    }

    std::streambuf* oldCout = nullptr;
    std::ofstream outputFile;
    std::ostringstream captured;

    if (capturedOutput) {
        oldCout = std::cout.rdbuf(captured.rdbuf());
    } else if (!outputFilePath.empty()) {
        outputFile.open(outputFilePath, std::ios::trunc);
        if (!outputFile) {
            ConsoleStyle::writeError("Could not open redirect file: " + outputFilePath + "\n");
            return false;
        }
        oldCout = std::cout.rdbuf(outputFile.rdbuf());
    }

    info->handler(context_, command.args);

    if (oldCout) {
        std::cout.rdbuf(oldCout);
    }
    if (capturedOutput) {
        *capturedOutput = captured.str();
    }

    return true;
}

std::string Shell::expandAlias(const std::string& input) const {
    size_t endPos = 0;
    std::string token = firstToken(input, endPos);
    if (token.empty() || token == "alias" || token == "unalias") {
        return input;
    }

    auto it = context_.aliases.find(token);
    if (it == context_.aliases.end()) {
        return input;
    }

    return it->second + input.substr(endPos);
}
