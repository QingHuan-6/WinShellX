#include "commands/TaskListCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <windows.h>
#include <tlhelp32.h>

#include <iomanip>
#include <iostream>
#include <string>

std::string TaskListCommand::name() const { return "tasklist"; }
std::string TaskListCommand::usage() const { return "tasklist"; }
std::string TaskListCommand::description() const { return "List running processes"; }

void TaskListCommand::execute(ShellContext&, const std::vector<std::string>&) const {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        ConsoleStyle::writeError("tasklist failed: " + getLastErrorMessage() + "\n");
        return;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(snapshot, &entry)) {
        ConsoleStyle::writeError("tasklist failed: " + getLastErrorMessage() + "\n");
        CloseHandle(snapshot);
        return;
    }

    std::cout << std::left << std::setw(36) << "Image Name"
              << std::right << std::setw(10) << "PID"
              << std::setw(12) << "Threads" << "\n";
    std::cout << std::string(58, '-') << "\n";

    do {
        std::cout << std::left << std::setw(36) << entry.szExeFile
                  << std::right << std::setw(10) << entry.th32ProcessID
                  << std::setw(12) << entry.cntThreads << "\n";
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
}
