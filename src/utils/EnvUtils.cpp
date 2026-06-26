#include "utils/EnvUtils.h"

#include <windows.h>

#include <string>

std::string expandEnvironmentVariables(const std::string& input) {
    if (input.empty()) {
        return input;
    }

    DWORD required = ExpandEnvironmentStringsA(input.c_str(), nullptr, 0);
    if (required == 0) {
        return input;
    }

    std::string expanded(required, '\0');
    DWORD written = ExpandEnvironmentStringsA(input.c_str(), expanded.data(), required);
    if (written == 0 || written > required) {
        return input;
    }

    if (written > 0) {
        expanded.resize(written - 1);
    }
    return expanded;
}
