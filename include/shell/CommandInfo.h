#pragma once

#include "shell/ShellContext.h"

#include <functional>
#include <string>
#include <vector>

struct CommandInfo {
    std::string usage;
    std::string description;
    std::function<void(ShellContext&, const std::vector<std::string>&)> handler;
};
