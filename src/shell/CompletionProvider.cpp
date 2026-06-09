#include "shell/CompletionProvider.h"

#include "utils/StringUtils.h"

#include <windows.h>

#include <algorithm>

namespace {
bool startsWithIgnoreCase(const std::string& value, const std::string& prefix) {
    std::string lowerValue = toLower(value);
    std::string lowerPrefix = toLower(prefix);
    return lowerValue.size() >= lowerPrefix.size() &&
           lowerValue.compare(0, lowerPrefix.size(), lowerPrefix) == 0;
}

std::string longestCommonPrefix(const std::vector<std::string>& values) {
    if (values.empty()) {
        return "";
    }

    std::string prefix = values.front();
    for (const std::string& value : values) {
        size_t index = 0;
        while (index < prefix.size() &&
               index < value.size() &&
               std::tolower(static_cast<unsigned char>(prefix[index])) ==
                   std::tolower(static_cast<unsigned char>(value[index]))) {
            ++index;
        }
        prefix.resize(index);
    }

    return prefix;
}

size_t lastSeparatorPosition(const std::string& value) {
    size_t slash = value.find_last_of("\\/");
    return slash == std::string::npos ? 0 : slash + 1;
}

std::string quoteIfNeeded(const std::string& value) {
    if (value.find(' ') == std::string::npos) {
        return value;
    }
    return "\"" + value + "\"";
}
}

CompletionProvider::CompletionProvider(std::vector<std::string> commandNames)
    : commandNames_(std::move(commandNames)) {
}

std::string CompletionProvider::complete(const std::string& input, const std::vector<std::string>& history) const {
    if (!input.empty()) {
        for (auto it = history.rbegin(); it != history.rend(); ++it) {
            if (*it != input && startsWithIgnoreCase(*it, input)) {
                return *it;
            }
        }
    }

    if (input.find(' ') == std::string::npos && input.find('\t') == std::string::npos) {
        return completeCommandName(input);
    }

    return completePath(input);
}

std::string CompletionProvider::completeCommandName(const std::string& input) const {
    if (input.empty()) {
        return "";
    }

    std::vector<std::string> matches;
    for (const std::string& name : commandNames_) {
        if (startsWithIgnoreCase(name, input)) {
            matches.push_back(name);
        }
    }

    if (matches.empty()) {
        return "";
    }

    std::string common = longestCommonPrefix(matches);
    return common.size() > input.size() ? common : matches.front();
}

std::string CompletionProvider::completePath(const std::string& input) const {
    size_t argStart = input.find(' ');
    if (argStart == std::string::npos) {
        return "";
    }

    std::string prefix = input.substr(0, argStart + 1);
    std::string pathPart = input.substr(argStart + 1);
    bool quoted = !pathPart.empty() && pathPart.front() == '"';
    if (quoted) {
        pathPart.erase(pathPart.begin());
    }

    size_t nameStart = lastSeparatorPosition(pathPart);
    std::string directory = nameStart == 0 ? "." : pathPart.substr(0, nameStart);
    std::string namePrefix = pathPart.substr(nameStart);
    std::string searchBase = directory;
    if (searchBase.empty()) {
        searchBase = ".";
    }

    std::string pattern = searchBase;
    if (!pattern.empty() && pattern.back() != '\\' && pattern.back() != '/') {
        pattern += "\\";
    }
    pattern += "*";

    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA(pattern.c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE) {
        return "";
    }

    std::vector<std::string> matches;
    do {
        std::string name = data.cFileName;
        if (name == "." || name == ".." || !startsWithIgnoreCase(name, namePrefix)) {
            continue;
        }

        std::string completedPath = nameStart == 0 ? name : pathPart.substr(0, nameStart) + name;
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            completedPath += "\\";
        }
        matches.push_back(completedPath);
    } while (FindNextFileA(handle, &data));

    FindClose(handle);

    if (matches.empty()) {
        return "";
    }

    std::string common = longestCommonPrefix(matches);
    std::string completed = common.size() > pathPart.size() ? common : matches.front();
    return prefix + quoteIfNeeded(completed);
}
