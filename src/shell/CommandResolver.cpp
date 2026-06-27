#include "shell/CommandResolver.h"

#include <windows.h>

#include <algorithm>
#include <cctype>
#include <sstream>

namespace {
struct CommandParts {
    std::string executable;
    std::string arguments;
};

std::string toLowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::vector<std::string> splitBySemicolon(const std::string& value) {
    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, ';')) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
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

bool hasDirectoryPart(const std::string& path) {
    return path.find('\\') != std::string::npos ||
           path.find('/') != std::string::npos ||
           (path.size() > 1 && path[1] == ':');
}

bool hasExtension(const std::string& path) {
    size_t slash = path.find_last_of("\\/");
    size_t dot = path.find_last_of('.');
    return dot != std::string::npos && (slash == std::string::npos || dot > slash);
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

bool isBatchFile(const std::string& path) {
    std::string lower = toLowerAscii(path);
    return lower.size() >= 4 &&
           (lower.substr(lower.size() - 4) == ".bat" ||
            lower.substr(lower.size() - 4) == ".cmd");
}

std::vector<std::string> executableNamesFromPathext(const std::string& executable) {
    if (hasExtension(executable)) {
        return {executable};
    }

    std::vector<std::string> result;
    std::string pathext = getEnvironmentValue("PATHEXT", ".COM;.EXE;.BAT;.CMD");
    for (const std::string& extension : splitBySemicolon(pathext)) {
        result.push_back(executable + extension);
    }

    return result;
}

std::vector<std::string> searchDirectories(bool executableHasDirectory) {
    if (executableHasDirectory) {
        return {""};
    }

    std::vector<std::string> directories;
    directories.push_back(".");

    char systemDirectory[MAX_PATH];
    if (GetSystemDirectoryA(systemDirectory, MAX_PATH) > 0) {
        directories.push_back(systemDirectory);
    }

    char windowsDirectory[MAX_PATH];
    if (GetWindowsDirectoryA(windowsDirectory, MAX_PATH) > 0) {
        directories.push_back(windowsDirectory);
    }

    std::string pathValue = getEnvironmentValue("PATH");
    for (const std::string& directory : splitBySemicolon(pathValue)) {
        directories.push_back(directory);
    }

    return directories;
}

CommandParts splitCommandLine(const std::string& commandLine) {
    CommandParts parts;
    size_t index = 0;

    while (index < commandLine.size() && std::isspace(static_cast<unsigned char>(commandLine[index]))) {
        ++index;
    }

    if (index >= commandLine.size()) {
        return parts;
    }

    if (commandLine[index] == '"') {
        ++index;
        size_t start = index;
        while (index < commandLine.size() && commandLine[index] != '"') {
            ++index;
        }
        parts.executable = commandLine.substr(start, index - start);
        if (index < commandLine.size() && commandLine[index] == '"') {
            ++index;
        }
    } else {
        size_t start = index;
        while (index < commandLine.size() && !std::isspace(static_cast<unsigned char>(commandLine[index]))) {
            ++index;
        }
        parts.executable = commandLine.substr(start, index - start);
    }

    while (index < commandLine.size() && std::isspace(static_cast<unsigned char>(commandLine[index]))) {
        ++index;
    }

    if (index < commandLine.size()) {
        parts.arguments = commandLine.substr(index);
    }

    return parts;
}
}

bool resolveCommand(const std::string& commandLine, ResolvedCommand& resolved) {
    CommandParts parts = splitCommandLine(commandLine);
    if (parts.executable.empty()) {
        return false;
    }

    bool executableHasDirectory = hasDirectoryPart(parts.executable);
    std::vector<std::string> names = executableNamesFromPathext(parts.executable);
    std::vector<std::string> directories = searchDirectories(executableHasDirectory);

    for (const std::string& directory : directories) {
        for (const std::string& name : names) {
            std::string candidate = executableHasDirectory ? name : joinPath(directory, name);
            if (fileExists(candidate)) {
                resolved.executablePath = candidate;
                resolved.arguments = parts.arguments;
                resolved.batchFile = isBatchFile(candidate);
                return true;
            }
        }
    }

    return false;
}

std::vector<std::string> resolveAllExecutablePaths(const std::string& commandName) {
    std::vector<std::string> matches;
    if (commandName.empty()) {
        return matches;
    }

    bool executableHasDirectory = hasDirectoryPart(commandName);
    std::vector<std::string> names = executableNamesFromPathext(commandName);
    std::vector<std::string> directories = searchDirectories(executableHasDirectory);

    std::vector<std::string> seenLower;
    for (const std::string& directory : directories) {
        for (const std::string& name : names) {
            std::string candidate = executableHasDirectory ? name : joinPath(directory, name);
            if (!fileExists(candidate)) {
                continue;
            }
            std::string lower = toLowerAscii(candidate);
            if (std::find(seenLower.begin(), seenLower.end(), lower) != seenLower.end()) {
                continue;
            }
            seenLower.push_back(lower);
            matches.push_back(candidate);
        }
    }

    return matches;
}

std::string commandNameFromLine(const std::string& commandLine) {
    return splitCommandLine(commandLine).executable;
}
