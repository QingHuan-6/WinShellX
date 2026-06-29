#include "commands/TopCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"
#include "utils/WinApiError.h"

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include <conio.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {
enum class SortMode {
    Cpu,
    Memory,
    Pid,
    Name,
    Threads
};

struct TopOptions {
    int rows = 20;
    int delayMs = 1000;
    SortMode sort = SortMode::Cpu;
    bool once = false;
};

struct ProcessSample {
    DWORD pid = 0;
    DWORD parentPid = 0;
    DWORD threads = 0;
    std::string name;
    ULONGLONG processTime = 0;
    SIZE_T workingSet = 0;
};

struct ProcessRow {
    DWORD pid = 0;
    DWORD parentPid = 0;
    DWORD threads = 0;
    std::string name;
    SIZE_T workingSet = 0;
    double cpuPercent = 0.0;
};

ULONGLONG fileTimeToUInt64(const FILETIME& value) {
    ULARGE_INTEGER integer;
    integer.LowPart = value.dwLowDateTime;
    integer.HighPart = value.dwHighDateTime;
    return integer.QuadPart;
}

ULONGLONG processTimeFor(HANDLE process) {
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if (!GetProcessTimes(process, &creationTime, &exitTime, &kernelTime, &userTime)) {
        return 0;
    }

    return fileTimeToUInt64(kernelTime) + fileTimeToUInt64(userTime);
}

ULONGLONG systemTime() {
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 0;
    }

    return fileTimeToUInt64(kernelTime) + fileTimeToUInt64(userTime);
}

SIZE_T workingSetFor(HANDLE process) {
    PROCESS_MEMORY_COUNTERS counters;
    if (!GetProcessMemoryInfo(process, &counters, sizeof(counters))) {
        return 0;
    }

    return counters.WorkingSetSize;
}

std::map<DWORD, ProcessSample> collectSamples() {
    std::map<DWORD, ProcessSample> samples;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return samples;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        return samples;
    }

    do {
        ProcessSample sample;
        sample.pid = entry.th32ProcessID;
        sample.parentPid = entry.th32ParentProcessID;
        sample.threads = entry.cntThreads;
        sample.name = entry.szExeFile;

        HANDLE process = OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
            FALSE,
            sample.pid);
        if (process != nullptr) {
            sample.processTime = processTimeFor(process);
            sample.workingSet = workingSetFor(process);
            CloseHandle(process);
        }

        samples[sample.pid] = sample;
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return samples;
}

std::string sortModeName(SortMode sort) {
    switch (sort) {
    case SortMode::Cpu:
        return "cpu";
    case SortMode::Memory:
        return "mem";
    case SortMode::Pid:
        return "pid";
    case SortMode::Name:
        return "name";
    case SortMode::Threads:
        return "threads";
    }
    return "cpu";
}

bool parsePositiveInt(const std::string& text, int& value) {
    if (text.empty()) {
        return false;
    }
    for (char ch : text) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }

    try {
        value = std::stoi(text);
    } catch (...) {
        return false;
    }
    return value > 0;
}

bool parseDelay(const std::string& text, int& delayMs) {
    try {
        double seconds = std::stod(text);
        if (seconds <= 0.0) {
            return false;
        }
        delayMs = static_cast<int>(seconds * 1000.0);
        delayMs = (std::max)(200, (std::min)(delayMs, 60000));
        return true;
    } catch (...) {
        return false;
    }
}

bool parseSortMode(const std::string& text, SortMode& sort) {
    std::string lowered = toLower(text);
    if (lowered == "cpu") {
        sort = SortMode::Cpu;
    } else if (lowered == "mem" || lowered == "memory") {
        sort = SortMode::Memory;
    } else if (lowered == "pid") {
        sort = SortMode::Pid;
    } else if (lowered == "name") {
        sort = SortMode::Name;
    } else if (lowered == "thread" || lowered == "threads") {
        sort = SortMode::Threads;
    } else {
        return false;
    }
    return true;
}

bool parseOptions(const std::vector<std::string>& args, TopOptions& options) {
    for (size_t i = 0; i < args.size(); ++i) {
        std::string arg = toLower(args[i]);
        if (arg == "-once" || arg == "/once") {
            options.once = true;
        } else if ((arg == "-n" || arg == "/n") && i + 1 < args.size()) {
            int rows = 0;
            if (!parsePositiveInt(args[++i], rows)) {
                ConsoleStyle::writeError("top: -n expects a positive integer.\n");
                return false;
            }
            options.rows = (std::max)(1, (std::min)(rows, 200));
        } else if ((arg == "-d" || arg == "/d") && i + 1 < args.size()) {
            if (!parseDelay(args[++i], options.delayMs)) {
                ConsoleStyle::writeError("top: -d expects a positive delay in seconds.\n");
                return false;
            }
        } else if ((arg == "-sort" || arg == "/sort") && i + 1 < args.size()) {
            if (!parseSortMode(args[++i], options.sort)) {
                ConsoleStyle::writeError("top: sort must be cpu, mem, pid, name, or threads.\n");
                return false;
            }
        } else {
            ConsoleStyle::writeError("Usage: top [-n rows] [-d seconds] [-sort cpu|mem|pid|name|threads] [-once]\n");
            return false;
        }
    }
    return true;
}

std::vector<ProcessRow> buildRows(
    const std::map<DWORD, ProcessSample>& before,
    const std::map<DWORD, ProcessSample>& after,
    ULONGLONG systemDelta) {
    std::vector<ProcessRow> rows;
    rows.reserve(after.size());

    for (const auto& item : after) {
        const ProcessSample& current = item.second;
        ProcessRow row;
        row.pid = current.pid;
        row.parentPid = current.parentPid;
        row.threads = current.threads;
        row.name = current.name;
        row.workingSet = current.workingSet;

        auto previous = before.find(current.pid);
        if (previous != before.end() && systemDelta > 0 &&
            current.processTime >= previous->second.processTime) {
            ULONGLONG processDelta = current.processTime - previous->second.processTime;
            row.cpuPercent = static_cast<double>(processDelta) * 100.0 /
                             static_cast<double>(systemDelta);
        }

        rows.push_back(row);
    }

    return rows;
}

void sortRows(std::vector<ProcessRow>& rows, SortMode sort) {
    std::sort(rows.begin(), rows.end(), [sort](const ProcessRow& left, const ProcessRow& right) {
        switch (sort) {
        case SortMode::Cpu:
            if (left.cpuPercent != right.cpuPercent) {
                return left.cpuPercent > right.cpuPercent;
            }
            return left.workingSet > right.workingSet;
        case SortMode::Memory:
            if (left.workingSet != right.workingSet) {
                return left.workingSet > right.workingSet;
            }
            return left.cpuPercent > right.cpuPercent;
        case SortMode::Pid:
            return left.pid < right.pid;
        case SortMode::Name:
            if (toLower(left.name) != toLower(right.name)) {
                return toLower(left.name) < toLower(right.name);
            }
            return left.pid < right.pid;
        case SortMode::Threads:
            if (left.threads != right.threads) {
                return left.threads > right.threads;
            }
            return left.pid < right.pid;
        }
        return left.pid < right.pid;
    });
}

void clearConsole() {
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(output, &info)) {
        std::cout << "\n\n";
        return;
    }

    DWORD cells = static_cast<DWORD>(info.dwSize.X) * static_cast<DWORD>(info.dwSize.Y);
    COORD home = {0, 0};
    DWORD written = 0;
    FillConsoleOutputCharacterA(output, ' ', cells, home, &written);
    FillConsoleOutputAttribute(output, info.wAttributes, cells, home, &written);
    SetConsoleCursorPosition(output, home);
}

std::string formatBytesMb(SIZE_T bytes) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1)
           << (static_cast<double>(bytes) / 1024.0 / 1024.0);
    return stream.str();
}

void printRows(const std::vector<ProcessRow>& rows, const TopOptions& options) {
    ConsoleStyle::writeInfo("WinShellX top - sort: " + sortModeName(options.sort) +
                            ", delay: " + std::to_string(options.delayMs) +
                            " ms, rows: " + std::to_string(options.rows) + "\n");
    ConsoleStyle::writeInfo("Keys: q quit, c CPU, m memory, p PID, n name, t threads\n");

    std::cout << std::left << std::setw(28) << "Image Name"
              << std::right << std::setw(8) << "PID"
              << std::setw(8) << "PPID"
              << std::setw(9) << "CPU%"
              << std::setw(12) << "Memory(MB)"
              << std::setw(10) << "Threads" << "\n";
    std::cout << std::string(75, '-') << "\n";

    int count = (std::min)(options.rows, static_cast<int>(rows.size()));
    for (int i = 0; i < count; ++i) {
        const ProcessRow& row = rows[static_cast<size_t>(i)];
        std::string name = row.name.size() > 27 ? row.name.substr(0, 27) : row.name;
        std::cout << std::left << std::setw(28) << name
                  << std::right << std::setw(8) << row.pid
                  << std::setw(8) << row.parentPid
                  << std::setw(8) << std::fixed << std::setprecision(1) << row.cpuPercent
                  << "%"
                  << std::setw(12) << formatBytesMb(row.workingSet)
                  << std::setw(10) << row.threads << "\n";
    }
}

bool handleKey(TopOptions& options) {
    if (!_kbhit()) {
        return true;
    }

    int key = _getch();
    if (key == 0 || key == 224) {
        if (_kbhit()) {
            _getch();
        }
        return true;
    }

    char ch = static_cast<char>(std::tolower(key));
    if (ch == 'q') {
        return false;
    }
    if (ch == 'c') {
        options.sort = SortMode::Cpu;
    } else if (ch == 'm') {
        options.sort = SortMode::Memory;
    } else if (ch == 'p') {
        options.sort = SortMode::Pid;
    } else if (ch == 'n') {
        options.sort = SortMode::Name;
    } else if (ch == 't') {
        options.sort = SortMode::Threads;
    }
    return true;
}
}

std::string TopCommand::name() const { return "top"; }
std::string TopCommand::usage() const { return "top [-n rows] [-d seconds] [-sort cpu|mem|pid|name|threads] [-once]"; }
std::string TopCommand::description() const { return "Interactively monitor processes by CPU and memory"; }

void TopCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    TopOptions options;
    if (!parseOptions(args, options)) {
        return;
    }

    std::map<DWORD, ProcessSample> previous = collectSamples();
    ULONGLONG previousSystem = systemTime();

    if (previous.empty()) {
        ConsoleStyle::writeError("top failed: " + getLastErrorMessage() + "\n");
        return;
    }

    bool running = true;
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(options.delayMs));

        std::map<DWORD, ProcessSample> current = collectSamples();
        ULONGLONG currentSystem = systemTime();
        ULONGLONG systemDelta = currentSystem >= previousSystem ? currentSystem - previousSystem : 0;

        std::vector<ProcessRow> rows = buildRows(previous, current, systemDelta);
        sortRows(rows, options.sort);

        if (!options.once) {
            clearConsole();
        }
        printRows(rows, options);

        if (options.once) {
            break;
        }

        previous = std::move(current);
        previousSystem = currentSystem;
        running = handleKey(options);
    }
}
