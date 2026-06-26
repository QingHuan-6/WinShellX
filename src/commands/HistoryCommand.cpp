#include "commands/HistoryCommand.h"

#include <iomanip>
#include <iostream>

std::string HistoryCommand::name() const { return "history"; }
std::string HistoryCommand::usage() const { return "history"; }
std::string HistoryCommand::description() const { return "Show command history"; }

void HistoryCommand::execute(ShellContext& context, const std::vector<std::string>&) const {
    for (size_t i = 0; i < context.history.size(); ++i) {
        std::cout << std::setw(4) << (i + 1) << "  " << context.history[i] << "\n";
    }
}
