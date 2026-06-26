#include "shell/Shell.h"

#include "commands/BuiltInCommands.h"
#include "shell/CommandParser.h"
#include "shell/ShellInputParser.h"
#include "utils/ConsoleStyle.h"
#include "utils/EnvUtils.h"
#include "utils/PathUtils.h"
#include "utils/StringUtils.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {
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
        executeInput(expandEnvironmentVariables(expandAlias(input)));
    }

    aliasStore_.save(context_.aliases);
    historyStore_.save(context_.history);
}

bool Shell::executeInput(const std::string& input) {
    ShellInputPlan plan = parseShellInput(input);
    if (plan.pipeline.empty()) {
        return false;
    }

    if (plan.background && (plan.pipeline.size() > 1 || !plan.outputFile.empty())) {
        ConsoleStyle::writeError("Background mode does not support pipes or redirection yet.\n");
        return false;
    }

    std::string pipeText;
    const std::string* stdinPtr = nullptr;

    for (size_t i = 0; i < plan.pipeline.size(); ++i) {
        const bool isLast = i + 1 == plan.pipeline.size();
        if (!isLast) {
            if (!executeSingle(plan.pipeline[i], stdinPtr, &pipeText, "")) {
                return false;
            }
            stdinPtr = &pipeText;
            continue;
        }

        return executeSingle(plan.pipeline[i], stdinPtr, nullptr, plan.outputFile, plan.background);
    }

    return true;
}

bool Shell::executeSingle(
    const std::string& input,
    const std::string* stdinText,
    std::string* capturedOutput,
    const std::string& outputFilePath,
    bool background) {
    ParsedCommand command = parseCommand(input);
    const ICommand* cmd = registry_.find(command.name);

    if (!cmd) {
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

    if (stdinText != nullptr && !stdinText->empty()) {
        ConsoleStyle::writeError(
            "Internal command '" + command.name + "' does not read piped input. "
            "Use an external filter such as find, e.g. tasklist | find \"chrome\".\n");
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

    cmd->execute(context_, command.args);

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
