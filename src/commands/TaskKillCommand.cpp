#include "commands/TaskKillCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <windows.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

namespace {
bool parsePid(const std::string& text, DWORD& pid) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](unsigned char ch) {
            return std::isdigit(ch);
        })) {
        return false;
    }

    unsigned long value = std::stoul(text);
    pid = static_cast<DWORD>(value);
    return true;
}
}

std::string TaskKillCommand::name() const { return "taskkill"; }
std::string TaskKillCommand::usage() const { return "taskkill <pid>"; }
std::string TaskKillCommand::description() const { return "Terminate process by PID"; }

void TaskKillCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.size() != 1) {
        ConsoleStyle::writeError("Usage: taskkill <pid>\n");
        return;
    }

    DWORD pid = 0;
    if (!parsePid(args[0], pid)) {
        ConsoleStyle::writeError("Invalid PID: " + args[0] + "\n");
        return;
    }

    HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!process) {
        ConsoleStyle::writeError("OpenProcess failed: " + getLastErrorMessage() + "\n");
        return;
    }

    if (!TerminateProcess(process, 1)) {
        ConsoleStyle::writeError("TerminateProcess failed: " + getLastErrorMessage() + "\n");
        CloseHandle(process);
        return;
    }

    CloseHandle(process);
    std::cout << "Process " << pid << " terminated.\n";
}
