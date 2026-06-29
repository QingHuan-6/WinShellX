#pragma once

#include <string>

class ShellExecutor {
public:
    virtual ~ShellExecutor() = default;
    virtual bool executeScriptLine(const std::string& input) = 0;
};
