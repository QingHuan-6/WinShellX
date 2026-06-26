#include "commands/DirCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/PathUtils.h"
#include "utils/StringUtils.h"
#include "utils/WinApiError.h"

#include <windows.h>

#include <iomanip>
#include <iostream>
#include <string>

std::string DirCommand::name() const { return "dir"; }
std::string DirCommand::usage() const { return "dir [path]"; }
std::string DirCommand::description() const { return "List files and directories"; }

void DirCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
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
