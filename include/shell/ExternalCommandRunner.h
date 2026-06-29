#pragma once

#include <string>

class JobManager;

struct ExternalRunOptions {
    const std::string* stdinText = nullptr;
    std::string* capturedOutput = nullptr;
    std::string outputFilePath;
    bool waitForExit = true;
    JobManager* jobManager = nullptr;
    std::string displayCommandLine;
};

class ExternalCommandRunner {
public:
    bool run(const std::string& commandLine, const ExternalRunOptions& options = {}) const;
};
