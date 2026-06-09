#include "shell/ExternalCommandRunner.h"

#include "utils/WinApiError.h"

#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

bool ExternalCommandRunner::run(const std::string& commandLine) const {
    if (commandLine.empty()) {
        return false;
    }

    std::vector<char> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back('\0');

    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInfo, sizeof(processInfo));
    startupInfo.cb = sizeof(startupInfo);

    BOOL created = CreateProcessA(
        nullptr,
        mutableCommand.data(),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo);

    if (!created) {
        std::cerr << "Unknown command or failed to start external program: "
                  << commandLine << "\n";
        std::cerr << "CreateProcess failed: " << getLastErrorMessage() << "\n";
        return false;
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    return true;
}
