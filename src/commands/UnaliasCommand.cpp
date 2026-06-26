#include "commands/UnaliasCommand.h"

#include "utils/ConsoleStyle.h"

#include <iostream>
#include <string>

std::string UnaliasCommand::name() const { return "unalias"; }
std::string UnaliasCommand::usage() const { return "unalias <name>"; }
std::string UnaliasCommand::description() const { return "Remove command alias"; }

void UnaliasCommand::execute(ShellContext& context, const std::vector<std::string>& args) const {
    if (args.size() != 1) {
        ConsoleStyle::writeError("Usage: unalias <name>\n");
        return;
    }

    size_t removed = context.aliases.erase(args[0]);
    if (removed == 0) {
        ConsoleStyle::writeError("Alias not found: " + args[0] + "\n");
        return;
    }

    ConsoleStyle::writeSuccess("Alias removed: " + args[0] + "\n");
}
