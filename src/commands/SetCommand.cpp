#include "commands/SetCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"
#include "utils/WinApiError.h"

#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

std::string SetCommand::name() const { return "set"; }
std::string SetCommand::usage() const { return "set [NAME=VALUE]"; }
std::string SetCommand::description() const { return "Show or set environment variables"; }

namespace {
std::vector<std::string> splitOnFirstEquals(const std::string& value) {
    size_t pos = value.find('=');
    if (pos == std::string::npos) {
        return {value};
    }
    return {value.substr(0, pos), value.substr(pos + 1)};
}

std::string readEnvironmentValue(const std::string& name) {
    DWORD size = GetEnvironmentVariableA(name.c_str(), nullptr, 0);
    if (size == 0) {
        return "";
    }
    std::vector<char> buffer(size);
    DWORD written = GetEnvironmentVariableA(name.c_str(), buffer.data(), size);
    if (written == 0 || written >= size) {
        return "";
    }
    return std::string(buffer.data(), written);
}
}

void SetCommand::execute(ShellContext&, const std::vector<std::string>& args) const {
    if (args.empty()) {
        LPTCH envBlock = GetEnvironmentStringsA();
        if (envBlock == nullptr) {
            return;
        }

        LPSTR cursor = reinterpret_cast<LPSTR>(envBlock);
        while (*cursor != '\0') {
            std::string entry(cursor);
            std::cout << entry << "\n";
            cursor += entry.size() + 1;
        }
        FreeEnvironmentStringsA(envBlock);
        return;
    }

    std::string joined = joinArgs(args);
    std::vector<std::string> parts = splitOnFirstEquals(joined);
    if (parts.size() == 1) {
        std::string value = readEnvironmentValue(parts[0]);
        if (value.empty()) {
            ConsoleStyle::writeError("Environment variable " + parts[0] + " is not defined.\n");
            return;
        }
        std::cout << parts[0] << "=" << value << "\n";
        return;
    }

    if (parts[0].empty()) {
        ConsoleStyle::writeError("set: invalid variable name.\n");
        return;
    }

    if (!SetEnvironmentVariableA(parts[0].c_str(), parts[1].c_str())) {
        ConsoleStyle::writeError("set failed: " + getLastErrorMessage() + "\n");
    }
}
