#pragma once

#include "shell/CommandInfo.h"

#include <map>
#include <string>
#include <vector>

// 内置命令注册表：维护命令名到 CommandInfo 的映射，供 Shell 查找与执行。
class CommandRegistry {
public:
    // 注册命令；name 会转为小写后存入，同名命令会被覆盖。
    void add(const std::string& name, CommandInfo command);

    // 按名称查找命令（忽略大小写）；未找到返回 nullptr。
    const CommandInfo* find(const std::string& name) const;

    // 返回所有已注册命令的名称，用于 Tab 补全等。
    std::vector<std::string> names() const;

    // 打印 help 命令输出的用法与说明列表。
    void printHelp() const;

private:
    std::map<std::string, CommandInfo> commands_;
};
