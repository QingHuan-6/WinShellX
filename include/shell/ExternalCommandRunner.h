#pragma once

#include <string>

struct ExternalRunOptions {
    const std::string* stdinText = nullptr;
    std::string* capturedOutput = nullptr;
    std::string outputFilePath;
};

class ExternalCommandRunner {
public:
    bool run(const std::string& commandLine, const ExternalRunOptions& options = {}) const;
};
