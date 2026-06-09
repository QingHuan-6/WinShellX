#include "utils/PathUtils.h"

#include <windows.h>

#include <iostream>

std::string getCurrentDirectoryText() {
    char buffer[MAX_PATH];
    DWORD length = GetCurrentDirectoryA(MAX_PATH, buffer);
    if (length == 0 || length >= MAX_PATH) {
        return ".";
    }
    return std::string(buffer);
}

std::string buildSearchPattern(const std::string& path) {
    std::string pattern = path.empty() ? "." : path;
    char last = pattern.back();
    if (last != '\\' && last != '/') {
        pattern += "\\";
    }
    pattern += "*";
    return pattern;
}

void printDiskSpace(const std::string& path) {
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalBytes;
    ULARGE_INTEGER totalFreeBytes;

    std::string target = path.empty() ? "." : path;
    if (GetDiskFreeSpaceExA(target.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        std::cout << "\nTotal bytes: " << totalBytes.QuadPart
                  << "\nFree bytes : " << totalFreeBytes.QuadPart << "\n";
    }
}
