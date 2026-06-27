#include "commands/TypeCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"

#include <fstream>
#include <iostream>
#include <string>

std::string TypeCommand::name() const { return "type"; }
std::string TypeCommand::usage() const { return "type <file>"; }
std::string TypeCommand::description() const { return "Display the contents of a text file"; }

void TypeCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.empty()) {
        ConsoleStyle::writeError("type: missing file name. Usage: " + usage() + "\n");
        return;
    }

    std::string path = joinArgs(args);
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        ConsoleStyle::writeError("type: cannot open " + path + "\n");
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::cout << content;
    if (content.empty() || content.back() != '\n') {
        std::cout << "\n";
    }
}
