#include "shell/ShellInputParser.h"

#include "utils/StringUtils.h"

namespace {
size_t findUnquoted(const std::string& input, char target, size_t from = 0) {
    bool inQuotes = false;
    for (size_t i = from; i < input.size(); ++i) {
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

std::vector<std::string> splitPipeline(const std::string& commandPart) {
    std::vector<std::string> parts;
    size_t segmentStart = 0;
    bool inQuotes = false;

    for (size_t i = 0; i < commandPart.size(); ++i) {
        if (commandPart[i] == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        if (!inQuotes && commandPart[i] == '|') {
            parts.push_back(trim(commandPart.substr(segmentStart, i - segmentStart)));
            segmentStart = i + 1;
        }
    }

    parts.push_back(trim(commandPart.substr(segmentStart)));
    return parts;
}
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

    plan.pipeline = splitPipeline(commandPart);
    if (plan.pipeline.size() == 1 && plan.pipeline.front().empty()) {
        plan.pipeline.clear();
    }

    return plan;
}
