#include "commands/MoveCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <windows.h>

std::string MoveCommand::name() const { return "move"; }
std::string MoveCommand::usage() const { return "move <source> <destination>"; }
std::string MoveCommand::description() const { return "Move or rename a file or directory"; }

void MoveCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.size() != 2) {
        ConsoleStyle::writeError("Usage: " + usage() + "\n");
        return;
    }

    if (!MoveFileExA(args[0].c_str(), args[1].c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) {
        ConsoleStyle::writeError("move failed: " + getLastErrorMessage() + "\n");
        return;
    }

    ConsoleStyle::writeSuccess("Moved: " + args[0] + " -> " + args[1] + "\n");
}
