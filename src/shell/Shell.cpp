#include "shell/Shell.h"

#include "commands/BuiltInCommands.h"
#include "shell/CommandParser.h"
#include "utils/PathUtils.h"
#include "utils/StringUtils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {
struct ShellInputPlan {
    std::string leftCommand;
    std::string rightCommand;
    std::string outputFile;
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
}

Shell::Shell() : historyStore_(".winshellx_history") {
    context_.historyFilePath = historyStore_.filePath();
    context_.history = historyStore_.load();
    registerBuiltInCommands(registry_);
}

void Shell::run() {
    std::cout << "WinShellX started. Type help for commands.\n";
    std::cout << "History loaded from " << context_.historyFilePath
              << ". Tab completes history/commands/paths, Up/Down browses history, F7 chooses recent history.\n";

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
        executeInput(input);
    }

    historyStore_.save(context_.history);
}

bool Shell::executeInput(const std::string& input) {
    ShellInputPlan plan = parseShellInput(input);
    if (plan.leftCommand.empty()) {
        return false;
    }

    if (!plan.rightCommand.empty()) {
        std::string pipeText;
        if (!executeSingle(plan.leftCommand, nullptr, &pipeText, "")) {
            return false;
        }
        return executeSingle(plan.rightCommand, &pipeText, nullptr, plan.outputFile);
    }

    return executeSingle(plan.leftCommand, nullptr, nullptr, plan.outputFile);
}

bool Shell::executeSingle(
    const std::string& input,
    const std::string* stdinText,
    std::string* capturedOutput,
    const std::string& outputFilePath) {
    ParsedCommand command = parseCommand(input);
    const CommandInfo* info = registry_.find(command.name);

    if (!info) {
        ExternalRunOptions options;
        options.stdinText = stdinText;
        options.capturedOutput = capturedOutput;
        options.outputFilePath = outputFilePath;
        return externalCommandRunner_.run(input, options);
    }

    std::streambuf* oldCout = nullptr;
    std::ofstream outputFile;
    std::ostringstream captured;

    if (capturedOutput) {
        oldCout = std::cout.rdbuf(captured.rdbuf());
    } else if (!outputFilePath.empty()) {
        outputFile.open(outputFilePath, std::ios::trunc);
        if (!outputFile) {
            std::cerr << "Could not open redirect file: " << outputFilePath << "\n";
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
