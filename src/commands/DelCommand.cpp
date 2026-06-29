#include "commands/DelCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"
#include "utils/WinApiError.h"

#include <windows.h>

std::string DelCommand::name() const { return "del"; }
std::string DelCommand::usage() const { return "del <file>"; }
std::string DelCommand::description() const { return "Delete a file"; }

void DelCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.empty()) {
        ConsoleStyle::writeError("del: missing file. Usage: " + usage() + "\n");
        return;
    }

    std::string path = joinArgs(args);
    if (!DeleteFileA(path.c_str())) {
        ConsoleStyle::writeError("del failed: " + getLastErrorMessage() + "\n");
        return;
    }

    ConsoleStyle::writeSuccess("Deleted file: " + path + "\n");
}
