#include "commands/BatCommand.h"

#include "shell/ShellExecutor.h"
#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"

#include <fstream>
#include <iostream>
#include <string>

namespace {
bool isCommentLine(const std::string& line) {
    std::string trimmed = trim(line);
    std::string lowered = toLower(trimmed);
    bool remComment = lowered.rfind("rem", 0) == 0 &&
                      (lowered.size() == 3 || std::isspace(static_cast<unsigned char>(lowered[3])));
    return remComment || trimmed.rfind("::", 0) == 0;
}
}

std::string BatCommand::name() const { return "bat"; }
std::string BatCommand::usage() const { return "bat <script.bat>"; }
std::string BatCommand::description() const { return "Run a simple line-by-line bat script"; }

void BatCommand::execute(ShellContext& context, const std::vector<std::string>& args) const {
    if (!context.executor) {
        ConsoleStyle::writeError("bat: shell executor is not available.\n");
        return;
    }
    if (args.empty()) {
        ConsoleStyle::writeError("bat: missing script file. Usage: " + usage() + "\n");
        return;
    }

    std::string path = joinArgs(args);
    std::ifstream file(path);
    if (!file) {
        ConsoleStyle::writeError("bat: cannot open " + path + "\n");
        return;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        std::string trimmed = trim(line);
        if (trimmed.empty() || isCommentLine(trimmed) || trimmed.rfind("@echo off", 0) == 0) {
            continue;
        }

        ConsoleStyle::writeInfo("bat:" + std::to_string(lineNumber) + "> " + trimmed + "\n");
        if (!context.executor->executeScriptLine(trimmed)) {
            ConsoleStyle::writeError("bat: command failed at line " + std::to_string(lineNumber) + "\n");
            return;
        }
        if (!context.running) {
            return;
        }
    }
}
