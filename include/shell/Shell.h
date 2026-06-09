#pragma once

#include "shell/CommandRegistry.h"
#include "shell/ExternalCommandRunner.h"
#include "shell/HistoryStore.h"
#include "shell/LineEditor.h"
#include "shell/ShellContext.h"

class Shell {
public:
    Shell();
    void run();

private:
    ShellContext context_;
    CommandRegistry registry_;
    ExternalCommandRunner externalCommandRunner_;
    HistoryStore historyStore_;
    LineEditor lineEditor_;
};
