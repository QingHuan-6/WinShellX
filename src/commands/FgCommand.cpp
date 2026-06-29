#include "commands/FgCommand.h"

#include "shell/JobManager.h"
#include "utils/ConsoleStyle.h"

#include <cctype>
#include <string>

namespace {
bool parseJobId(const std::string& text, int& id) {
    if (text.empty()) {
        return false;
    }

    size_t start = text[0] == '%' ? 1 : 0;
    if (start >= text.size()) {
        return false;
    }
    for (size_t i = start; i < text.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) {
            return false;
        }
    }

    try {
        id = std::stoi(text.substr(start));
    } catch (...) {
        return false;
    }
    return id > 0;
}
}

std::string FgCommand::name() const { return "fg"; }
std::string FgCommand::usage() const { return "fg <jobid>"; }
std::string FgCommand::description() const { return "Wait for a background job in foreground"; }

void FgCommand::execute(ShellContext& context, const std::vector<std::string>& args) const {
    if (!context.jobManager) {
        ConsoleStyle::writeError("fg: job manager is not available.\n");
        return;
    }
    if (args.size() != 1) {
        ConsoleStyle::writeError("Usage: fg <jobid>\n");
        return;
    }

    int id = 0;
    if (!parseJobId(args[0], id)) {
        ConsoleStyle::writeError("fg: job id must be a positive integer.\n");
        return;
    }

    context.jobManager->waitForeground(id);
}
