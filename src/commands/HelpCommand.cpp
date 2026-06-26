#include "commands/HelpCommand.h"

#include "shell/CommandRegistry.h"

std::string HelpCommand::name() const { return "help"; }
std::string HelpCommand::usage() const { return "help"; }
std::string HelpCommand::description() const { return "Show command help"; }

HelpCommand::HelpCommand(const CommandRegistry& registry) : registry_(registry) {}

void HelpCommand::execute(ShellContext&, const std::vector<std::string>&) const {
    registry_.printHelp();
}
