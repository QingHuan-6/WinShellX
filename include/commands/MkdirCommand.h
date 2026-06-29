#pragma once

#include "shell/ICommand.h"

class MkdirCommand : public ICommand {
public:
    std::string name() const override;
    std::string usage() const override;
    std::string description() const override;
    void execute(ShellContext& context, const std::vector<std::string>& args) const override;
};
