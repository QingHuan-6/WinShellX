#pragma once

#include <string>

struct ExternalRunOptions {
    const std::string* stdinText = nullptr;
    std::string* capturedOutput = nullptr;
    std::string outputFilePath;
    bool waitForExit = true;
};

class ExternalCommandRunner {
public:
    bool run(const std::string& commandLine, const ExternalRunOptions& options = {}) const;
};
