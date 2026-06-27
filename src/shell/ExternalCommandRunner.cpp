#include "shell/ExternalCommandRunner.h"

#include "shell/CommandResolver.h"
#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <windows.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {
std::string quote(const std::string& value) {
    return "\"" + value + "\"";
}

std::string getEnvironmentValue(const char* name, const std::string& fallback = "") {
    DWORD size = GetEnvironmentVariableA(name, nullptr, 0);
    if (size == 0) {
        return fallback;
    }

    std::vector<char> buffer(size);
    DWORD written = GetEnvironmentVariableA(name, buffer.data(), size);
    if (written == 0 || written >= size) {
        return fallback;
    }

    return std::string(buffer.data(), written);
}

bool fileExists(const std::string& path) {
    DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
           (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

std::string joinPath(const std::string& directory, const std::string& fileName) {
    if (directory.empty() || directory == ".") {
        return fileName;
    }

    char last = directory.back();
    if (last == '\\' || last == '/') {
        return directory + fileName;
    }

    return directory + "\\" + fileName;
}

std::string cmdExecutablePath() {
    std::string comspec = getEnvironmentValue("ComSpec");
    if (!comspec.empty() && fileExists(comspec)) {
        return comspec;
    }

    char systemDirectory[MAX_PATH];
    if (GetSystemDirectoryA(systemDirectory, MAX_PATH) > 0) {
        std::string candidate = joinPath(systemDirectory, "cmd.exe");
        if (fileExists(candidate)) {
            return candidate;
        }
    }

    return "cmd.exe";
}

std::string buildProcessCommandLine(const ResolvedCommand& command) {
    if (command.batchFile) {
        std::string line = quote(cmdExecutablePath()) + " /d /s /c \"";
        line += quote(command.executablePath);
        if (!command.arguments.empty()) {
            line += " " + command.arguments;
        }
        line += "\"";
        return line;
    }

    std::string line = quote(command.executablePath);
    if (!command.arguments.empty()) {
        line += " " + command.arguments;
    }
    return line;
}

std::string applicationNameFor(const ResolvedCommand& command) {
    return command.batchFile ? cmdExecutablePath() : command.executablePath;
}

void closeIfValid(HANDLE& handle) {
    if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        handle = nullptr;
    }
}

void appendFromPipe(HANDLE readHandle, std::string* output) {
    char buffer[4096];
    DWORD bytesRead = 0;
    while (ReadFile(readHandle, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0) {
        if (output) {
            output->append(buffer, buffer + bytesRead);
        }
    }
}
}

bool ExternalCommandRunner::run(const std::string& commandLine, const ExternalRunOptions& options) const {
    if (commandLine.empty()) {
        return false;
    }

    ResolvedCommand resolved;
    if (!resolveCommand(commandLine, resolved)) {
        ConsoleStyle::writeError("'" + commandNameFromLine(commandLine)
                  + "' is not recognized as an internal or external command.\n");
        return false;
    }

    std::string processCommandLine = buildProcessCommandLine(resolved);
    std::string applicationName = applicationNameFor(resolved);
    std::vector<char> mutableCommand(processCommandLine.begin(), processCommandLine.end());
    mutableCommand.push_back('\0');

    SECURITY_ATTRIBUTES securityAttributes;
    ZeroMemory(&securityAttributes, sizeof(securityAttributes));
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE stdinRead = nullptr;
    HANDLE stdinWrite = nullptr;
    HANDLE stdoutRead = nullptr;
    HANDLE stdoutWrite = nullptr;
    HANDLE outputFile = nullptr;

    bool redirectInput = options.stdinText != nullptr;
    bool captureOutput = options.capturedOutput != nullptr;
    bool redirectOutputFile = !options.outputFilePath.empty();

    if (redirectInput) {
        if (!CreatePipe(&stdinRead, &stdinWrite, &securityAttributes, 0)) {
            ConsoleStyle::writeError("CreatePipe failed: " + getLastErrorMessage() + "\n");
            return false;
        }
        SetHandleInformation(stdinWrite, HANDLE_FLAG_INHERIT, 0);
    }

    if (captureOutput) {
        if (!CreatePipe(&stdoutRead, &stdoutWrite, &securityAttributes, 0)) {
            ConsoleStyle::writeError("CreatePipe failed: " + getLastErrorMessage() + "\n");
            closeIfValid(stdinRead);
            closeIfValid(stdinWrite);
            return false;
        }
        SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0);
    } else if (redirectOutputFile) {
        outputFile = CreateFileA(
            options.outputFilePath.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ,
            &securityAttributes,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (outputFile == INVALID_HANDLE_VALUE) {
            ConsoleStyle::writeError("Could not open redirect file: " + getLastErrorMessage() + "\n");
            closeIfValid(stdinRead);
            closeIfValid(stdinWrite);
            return false;
        }
    }

    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInfo, sizeof(processInfo));
    startupInfo.cb = sizeof(startupInfo);
    if (redirectInput || captureOutput || redirectOutputFile) {
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;
        startupInfo.hStdInput = redirectInput ? stdinRead : GetStdHandle(STD_INPUT_HANDLE);
        startupInfo.hStdOutput = captureOutput ? stdoutWrite
                            : redirectOutputFile ? outputFile
                                                 : GetStdHandle(STD_OUTPUT_HANDLE);
        startupInfo.hStdError = startupInfo.hStdOutput;
    }

    BOOL created = CreateProcessA(
        applicationName.c_str(),
        mutableCommand.data(),
        nullptr,
        nullptr,
        (redirectInput || captureOutput || redirectOutputFile) ? TRUE : FALSE,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo);

    if (!created) {
        ConsoleStyle::writeError("Failed to start external program: "
                  + resolved.executablePath + "\n");
        ConsoleStyle::writeError("CreateProcess failed: " + getLastErrorMessage() + "\n");
        closeIfValid(stdinRead);
        closeIfValid(stdinWrite);
        closeIfValid(stdoutRead);
        closeIfValid(stdoutWrite);
        closeIfValid(outputFile);
        return false;
    }

    closeIfValid(stdinRead);
    closeIfValid(stdoutWrite);
    closeIfValid(outputFile);

    std::thread reader;
    if (captureOutput) {
        reader = std::thread(appendFromPipe, stdoutRead, options.capturedOutput);
    }

    if (redirectInput && options.stdinText) {
        DWORD bytesWritten = 0;
        WriteFile(stdinWrite, options.stdinText->data(), static_cast<DWORD>(options.stdinText->size()), &bytesWritten, nullptr);
        closeIfValid(stdinWrite);
    }

    if (options.waitForExit) {
        WaitForSingleObject(processInfo.hProcess, INFINITE);
        if (reader.joinable()) {
            reader.join();
        }
    } else {
        ConsoleStyle::writeSuccess("Started background process, PID: " +
                                   std::to_string(processInfo.dwProcessId) + "\n");
        if (reader.joinable()) {
            reader.detach();
        }
    }

    closeIfValid(stdoutRead);
    closeIfValid(stdinWrite);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    return true;
}
