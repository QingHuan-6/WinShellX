#include "commands/BuiltInCommands.h"

#include "utils/PathUtils.h"
#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"
#include "utils/WinApiError.h"

#include <windows.h>
#include <tlhelp32.h>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

static void commandCd(ShellContext&, const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << getCurrentDirectoryText() << "\n";
        return;
    }

    std::string path = joinArgs(args);
    if (!SetCurrentDirectoryA(path.c_str())) {
        ConsoleStyle::writeError("cd failed: " + getLastErrorMessage() + "\n");
    }
}

static void commandDir(ShellContext&, const std::vector<std::string>& args) {
    std::string path = args.empty() ? "." : joinArgs(args);
    std::string pattern = buildSearchPattern(path);

    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA(pattern.c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE) {
        ConsoleStyle::writeError("dir failed: " + getLastErrorMessage() + "\n");
        return;
    }

    std::cout << " Directory of " << path << "\n\n";

    unsigned long long fileCount = 0;
    unsigned long long dirCount = 0;
    unsigned long long totalSize = 0;

    do {
        bool isDir = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        ULARGE_INTEGER size;
        size.HighPart = data.nFileSizeHigh;
        size.LowPart = data.nFileSizeLow;

        SYSTEMTIME utcTime;
        SYSTEMTIME localTime;
        FileTimeToSystemTime(&data.ftLastWriteTime, &utcTime);
        SystemTimeToTzSpecificLocalTime(nullptr, &utcTime, &localTime);

        std::cout << std::right << std::setfill('0')
                  << std::setw(4) << localTime.wYear << "-"
                  << std::setw(2) << localTime.wMonth << "-"
                  << std::setw(2) << localTime.wDay << " "
                  << std::setw(2) << localTime.wHour << ":"
                  << std::setw(2) << localTime.wMinute
                  << std::setfill(' ') << "  ";

        if (isDir) {
            ++dirCount;
            std::cout << std::setw(12) << "<DIR>";
        } else {
            ++fileCount;
            totalSize += size.QuadPart;
            std::cout << std::setw(12) << size.QuadPart;
        }

        std::cout << "  ";
        if (isDir) {
            ConsoleStyle::writeDirectoryName(data.cFileName);
            std::cout << "\n";
        } else {
            std::cout << data.cFileName << "\n";
        }
    } while (FindNextFileA(handle, &data));

    FindClose(handle);

    std::cout << "\n" << fileCount << " File(s), " << totalSize << " bytes\n";
    std::cout << dirCount << " Dir(s)\n";
    printDiskSpace(path);
}

static void commandHistory(ShellContext& context, const std::vector<std::string>&) {
    for (size_t i = 0; i < context.history.size(); ++i) {
        std::cout << std::setw(4) << (i + 1) << "  " << context.history[i] << "\n";
    }
}

static void commandExit(ShellContext& context, const std::vector<std::string>&) {
    context.running = false;
}

static void commandAlias(ShellContext& context, const std::vector<std::string>& args) {
    if (args.empty()) {
        if (context.aliases.empty()) {
            std::cout << "No aliases defined.\n";
            return;
        }

        for (const auto& item : context.aliases) {
            std::cout << item.first << "=" << item.second << "\n";
        }
        return;
    }

    std::string definition = joinArgs(args);
    size_t equals = definition.find('=');
    if (equals == std::string::npos) {
        auto it = context.aliases.find(definition);
        if (it == context.aliases.end()) {
            ConsoleStyle::writeError("Alias not found: " + definition + "\n");
            return;
        }
        std::cout << it->first << "=" << it->second << "\n";
        return;
    }

    std::string name = trim(definition.substr(0, equals));
    std::string value = trim(definition.substr(equals + 1));
    if (name.empty() || value.empty()) {
        ConsoleStyle::writeError("Usage: alias name=command\n");
        return;
    }

    context.aliases[name] = value;
    ConsoleStyle::writeSuccess("Alias set: " + name + "=" + value + "\n");
}

static void commandUnalias(ShellContext& context, const std::vector<std::string>& args) {
    if (args.size() != 1) {
        ConsoleStyle::writeError("Usage: unalias <name>\n");
        return;
    }

    size_t removed = context.aliases.erase(args[0]);
    if (removed == 0) {
        ConsoleStyle::writeError("Alias not found: " + args[0] + "\n");
        return;
    }

    ConsoleStyle::writeSuccess("Alias removed: " + args[0] + "\n");
}

static void commandCls(ShellContext&, const std::vector<std::string>&) {
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (outputHandle == INVALID_HANDLE_VALUE) {
        ConsoleStyle::writeError("cls failed: invalid console handle\n");
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    if (!GetConsoleScreenBufferInfo(outputHandle, &bufferInfo)) {
        ConsoleStyle::writeError("cls failed: " + getLastErrorMessage() + "\n");
        return;
    }

    DWORD cellCount = static_cast<DWORD>(bufferInfo.dwSize.X) * static_cast<DWORD>(bufferInfo.dwSize.Y);
    COORD home = {0, 0};
    DWORD cellsWritten = 0;

    if (!FillConsoleOutputCharacterA(outputHandle, ' ', cellCount, home, &cellsWritten)) {
        ConsoleStyle::writeError("cls failed: " + getLastErrorMessage() + "\n");
        return;
    }

    if (!FillConsoleOutputAttribute(outputHandle, bufferInfo.wAttributes, cellCount, home, &cellsWritten)) {
        ConsoleStyle::writeError("cls failed: " + getLastErrorMessage() + "\n");
        return;
    }

    SetConsoleCursorPosition(outputHandle, home);
}

static void commandTaskList(ShellContext&, const std::vector<std::string>&) {
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

static bool parsePid(const std::string& text, DWORD& pid) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](unsigned char ch) {
            return std::isdigit(ch);
        })) {
        return false;
    }

    unsigned long value = std::stoul(text);
    pid = static_cast<DWORD>(value);
    return true;
}

static void commandTaskKill(ShellContext&, const std::vector<std::string>& args) {
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

void registerBuiltInCommands(CommandRegistry& registry) {
    registry.add("alias", {"alias [name=command]", "Create or list command aliases", commandAlias});
    registry.add("cd", {"cd <path>", "Change current directory", commandCd});
    registry.add("cls", {"cls", "Clear console screen", commandCls});
    registry.add("dir", {"dir [path]", "List files and directories", commandDir});
    registry.add("history", {"history", "Show command history", commandHistory});
    registry.add("exit", {"exit", "Exit WinShellX", commandExit});
    registry.add("tasklist", {"tasklist", "List running processes", commandTaskList});
    registry.add("taskkill", {"taskkill <pid>", "Terminate process by PID", commandTaskKill});
    registry.add("unalias", {"unalias <name>", "Remove command alias", commandUnalias});
    registry.add("help", {"help", "Show command help", [&registry](ShellContext&, const std::vector<std::string>&) {
        registry.printHelp();
    }});
}
