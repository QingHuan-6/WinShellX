#pragma once

#include <string>

class ExternalCommandRunner {
public:
    bool run(const std::string& commandLine) const;
};
