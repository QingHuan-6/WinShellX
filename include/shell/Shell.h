#pragma once

#include "shell/CommandRegistry.h"
#include "shell/ExternalCommandRunner.h"
#include "shell/AliasStore.h"
#include "shell/HistoryStore.h"
#include "shell/JobManager.h"
#include "shell/LineEditor.h"
#include "shell/ShellExecutor.h"
#include "shell/ShellContext.h"

#include <string>

class Shell : public ShellExecutor {
public:
    Shell();
    void run();
    bool executeScriptLine(const std::string& input) override;

private:
    bool executeInput(const std::string& input);

    bool executeSingle(
        const std::string& input,
        const std::string* stdinText,
        std::string* capturedOutput,
        const std::string& outputFilePath,
        bool background = false);

    std::string expandAlias(const std::string& input) const;

    ShellContext context_;
    CommandRegistry registry_;
    ExternalCommandRunner externalCommandRunner_;
    JobManager jobManager_;
    AliasStore aliasStore_;
    HistoryStore historyStore_;
    LineEditor lineEditor_;
};
