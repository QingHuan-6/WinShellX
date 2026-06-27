#include "commands/EchoCommand.h"

#include "utils/StringUtils.h"

#include <iostream>

std::string EchoCommand::name() const { return "echo"; }
std::string EchoCommand::usage() const { return "echo <text>"; }
std::string EchoCommand::description() const { return "Print the given text"; }

void EchoCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    std::cout << joinArgs(args) << "\n";
}
