#include "commands/RmdirCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"
#include "utils/WinApiError.h"

#include <windows.h>

std::string RmdirCommand::name() const { return "rmdir"; }
std::string RmdirCommand::usage() const { return "rmdir <directory>"; }
std::string RmdirCommand::description() const { return "Remove an empty directory"; }

void RmdirCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.empty()) {
        ConsoleStyle::writeError("rmdir: missing directory. Usage: " + usage() + "\n");
        return;
    }

    std::string path = joinArgs(args);
    if (!RemoveDirectoryA(path.c_str())) {
        ConsoleStyle::writeError("rmdir failed: " + getLastErrorMessage() + "\n");
        return;
    }

    ConsoleStyle::writeSuccess("Removed directory: " + path + "\n");
}
