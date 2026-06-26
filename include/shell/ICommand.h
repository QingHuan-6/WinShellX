#pragma once

#include "shell/ShellContext.h"

#include <string>
#include <vector>

// 内置命令统一接口：每个内部命令实现为一个 ICommand 子类。
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual std::string name() const = 0;
    virtual std::string usage() const = 0;
    virtual std::string description() const = 0;
    virtual void execute(ShellContext& context, const std::vector<std::string>& args) const = 0;
};
