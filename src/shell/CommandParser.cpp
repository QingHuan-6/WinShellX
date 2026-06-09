#include "shell/CommandParser.h"

#include "utils/StringUtils.h"

ParsedCommand parseCommand(const std::string& input) {
    std::vector<std::string> parts = splitArguments(input);
    ParsedCommand parsed;

    if (!parts.empty()) {
        parsed.name = toLower(parts.front());
        parsed.args.assign(parts.begin() + 1, parts.end());
    }

    return parsed;
}
