#pragma once

#include <string>
#include <vector>

struct ShellInputPlan {
    std::vector<std::string> pipeline;
    std::string outputFile;
    bool background = false;
};

ShellInputPlan parseShellInput(const std::string& input);
