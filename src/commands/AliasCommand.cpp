#include "commands/AliasCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/StringUtils.h"

#include <iostream>
#include <string>

std::string AliasCommand::name() const { return "alias"; }
std::string AliasCommand::usage() const { return "alias [name=command]"; }
std::string AliasCommand::description() const { return "Create or list command aliases"; }

void AliasCommand::execute(ShellContext& context, const std::vector<std::string>& args) const {
    if (args.empty()) {
        if (context.aliases.empty()) {
            std::cout << "No aliases defined.\n";
            return;
        }

        for (const auto& item : context.aliases) {
            std::cout << item.first << "=" << item.second << "\n";
        }
        return;
    }

    std::string definition = joinArgs(args);
    size_t equals = definition.find('=');
    if (equals == std::string::npos) {
        auto it = context.aliases.find(definition);
        if (it == context.aliases.end()) {
            ConsoleStyle::writeError("Alias not found: " + definition + "\n");
            return;
        }
        std::cout << it->first << "=" << it->second << "\n";
        return;
    }

    std::string name = trim(definition.substr(0, equals));
    std::string value = trim(definition.substr(equals + 1));
    if (name.empty() || value.empty()) {
        ConsoleStyle::writeError("Usage: alias name=command\n");
        return;
    }

    context.aliases[name] = value;
    ConsoleStyle::writeSuccess("Alias set: " + name + "=" + value + "\n");
}
