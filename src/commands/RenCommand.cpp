#include "commands/RenCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <windows.h>

std::string RenCommand::name() const { return "ren"; }
std::string RenCommand::usage() const { return "ren <old> <new>"; }
std::string RenCommand::description() const { return "Rename a file or directory"; }

void RenCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.size() != 2) {
        ConsoleStyle::writeError("Usage: " + usage() + "\n");
        return;
    }

    if (!MoveFileExA(args[0].c_str(), args[1].c_str(), MOVEFILE_REPLACE_EXISTING)) {
        ConsoleStyle::writeError("ren failed: " + getLastErrorMessage() + "\n");
        return;
    }

    ConsoleStyle::writeSuccess("Renamed: " + args[0] + " -> " + args[1] + "\n");
}
