#include "commands/BuiltInCommands.h"

#include "commands/AliasCommand.h"
#include "commands/CdCommand.h"
#include "commands/ClsCommand.h"
#include "commands/DirCommand.h"
#include "commands/ExitCommand.h"
#include "commands/HelpCommand.h"
#include "commands/HistoryCommand.h"
#include "commands/TaskKillCommand.h"
#include "commands/TaskListCommand.h"
#include "commands/UnaliasCommand.h"

#include <memory>

void registerBuiltInCommands(CommandRegistry& registry) {
    registry.add(std::make_unique<AliasCommand>());
    registry.add(std::make_unique<CdCommand>());
    registry.add(std::make_unique<ClsCommand>());
    registry.add(std::make_unique<DirCommand>());
    registry.add(std::make_unique<HistoryCommand>());
    registry.add(std::make_unique<ExitCommand>());
    registry.add(std::make_unique<TaskListCommand>());
    registry.add(std::make_unique<TaskKillCommand>());
    registry.add(std::make_unique<UnaliasCommand>());
    registry.add(std::make_unique<HelpCommand>(registry));
}
