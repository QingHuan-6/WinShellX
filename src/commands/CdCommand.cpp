#include "commands/CdCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/PathUtils.h"
#include "utils/StringUtils.h"
#include "utils/WinApiError.h"

#include <windows.h>

#include <iostream>
#include <string>

std::string CdCommand::name() const { return "cd"; }
std::string CdCommand::usage() const { return "cd <path>"; }
std::string CdCommand::description() const { return "Change current directory"; }

void CdCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.empty()) {
        std::cout << getCurrentDirectoryText() << "\n";
        return;
    }

    std::string path = joinArgs(args);
    if (!SetCurrentDirectoryA(path.c_str())) {
        ConsoleStyle::writeError("cd failed: " + getLastErrorMessage() + "\n");
    }
}
