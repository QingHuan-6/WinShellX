#pragma once

#include "shell/CommandRegistry.h"
#include "shell/ShellContext.h"

class Shell {
public:
    Shell();
    void run();

private:
    ShellContext context_;
    CommandRegistry registry_;
};
