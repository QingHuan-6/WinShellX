#include "commands/ExitCommand.h"

std::string ExitCommand::name() const { return "exit"; }
std::string ExitCommand::usage() const { return "exit"; }
std::string ExitCommand::description() const { return "Exit WinShellX"; }

void ExitCommand::execute(ShellContext& context, const std::vector<std::string>&) const {
    context.running = false;
}
