#include "commands/CopyCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <windows.h>

std::string CopyCommand::name() const { return "copy"; }
std::string CopyCommand::usage() const { return "copy <source> <destination>"; }
std::string CopyCommand::description() const { return "Copy a file"; }

void CopyCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.size() != 2) {
        ConsoleStyle::writeError("Usage: " + usage() + "\n");
        return;
    }

    if (!CopyFileA(args[0].c_str(), args[1].c_str(), FALSE)) {
        ConsoleStyle::writeError("copy failed: " + getLastErrorMessage() + "\n");
        return;
    }

    ConsoleStyle::writeSuccess("Copied file: " + args[0] + " -> " + args[1] + "\n");
}
