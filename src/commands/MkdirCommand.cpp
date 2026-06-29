#include "commands/MkdirCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"
#include "utils/WinApiError.h"

#include <windows.h>

std::string MkdirCommand::name() const { return "mkdir"; }
std::string MkdirCommand::usage() const { return "mkdir <directory>"; }
std::string MkdirCommand::description() const { return "Create a directory"; }

void MkdirCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.empty()) {
        ConsoleStyle::writeError("mkdir: missing directory. Usage: " + usage() + "\n");
        return;
    }

    std::string path = joinArgs(args);
    if (!CreateDirectoryA(path.c_str(), nullptr)) {
        ConsoleStyle::writeError("mkdir failed: " + getLastErrorMessage() + "\n");
        return;
    }

    ConsoleStyle::writeSuccess("Created directory: " + path + "\n");
}
