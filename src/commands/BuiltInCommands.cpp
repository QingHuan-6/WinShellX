#include "commands/BuiltInCommands.h"

#include "commands/AliasCommand.h"
#include "commands/BatCommand.h"
#include "commands/CdCommand.h"
#include "commands/ClsCommand.h"
#include "commands/CopyCommand.h"
#include "commands/DelCommand.h"
#include "commands/DirCommand.h"
#include "commands/EchoCommand.h"
#include "commands/ExitCommand.h"
#include "commands/FgCommand.h"
#include "commands/HelpCommand.h"
#include "commands/HistoryCommand.h"
#include "commands/JobsCommand.h"
#include "commands/KillJobCommand.h"
#include "commands/MkdirCommand.h"
#include "commands/MoveCommand.h"
#include "commands/RenCommand.h"
#include "commands/RmdirCommand.h"
#include "commands/SetCommand.h"
#include "commands/TaskKillCommand.h"
#include "commands/TaskListCommand.h"
#include "commands/TopCommand.h"
#include "commands/TypeCommand.h"
#include "commands/UnaliasCommand.h"
#include "commands/WhereCommand.h"

#include <memory>

void registerBuiltInCommands(CommandRegistry& registry) {
    registry.add(std::make_unique<AliasCommand>());
    registry.add(std::make_unique<BatCommand>());
    registry.add(std::make_unique<CdCommand>());
    registry.add(std::make_unique<ClsCommand>());
    registry.add(std::make_unique<CopyCommand>());
    registry.add(std::make_unique<DelCommand>());
    registry.add(std::make_unique<DirCommand>());
    registry.add(std::make_unique<EchoCommand>());
    registry.add(std::make_unique<FgCommand>());
    registry.add(std::make_unique<HistoryCommand>());
    registry.add(std::make_unique<ExitCommand>());
    registry.add(std::make_unique<JobsCommand>());
    registry.add(std::make_unique<KillJobCommand>());
    registry.add(std::make_unique<MkdirCommand>());
    registry.add(std::make_unique<MoveCommand>());
    registry.add(std::make_unique<RenCommand>());
    registry.add(std::make_unique<RmdirCommand>());
    registry.add(std::make_unique<TaskListCommand>());
    registry.add(std::make_unique<TaskKillCommand>());
    registry.add(std::make_unique<TopCommand>());
    registry.add(std::make_unique<UnaliasCommand>());
    registry.add(std::make_unique<SetCommand>());
    registry.add(std::make_unique<TypeCommand>());
    registry.add(std::make_unique<WhereCommand>());
    registry.add(std::make_unique<HelpCommand>(registry));
}
