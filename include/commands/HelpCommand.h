#pragma once

#include "shell/ICommand.h"

class CommandRegistry;

class HelpCommand : public ICommand {
public:
    explicit HelpCommand(const CommandRegistry& registry);

    std::string name() const override;
    std::string usage() const override;
    std::string description() const override;
    void execute(ShellContext& context, const std::vector<std::string>& args) const override;

private:
    const CommandRegistry& registry_;
};
