#pragma once

#include "shell/CommandRegistry.h"
#include "shell/ExternalCommandRunner.h"
#include "shell/HistoryStore.h"
#include "shell/LineEditor.h"
#include "shell/ShellContext.h"

#include <string>

class Shell {
public:
    Shell();
    void run();

private:
    bool executeInput(const std::string& input);
    bool executeSingle(
        const std::string& input,
        const std::string* stdinText,
        std::string* capturedOutput,
        const std::string& outputFilePath);

    ShellContext context_;
    CommandRegistry registry_;
    ExternalCommandRunner externalCommandRunner_;
    HistoryStore historyStore_;
    LineEditor lineEditor_;
};
