#pragma once

#include <map>
#include <string>
#include <vector>

struct ShellContext {
    // 运行状态
    bool running = true;
    // 历史记录
    std::vector<std::string> history;
    // 历史记录文件路径
    std::string historyFilePath;
    // 别名
    std::map<std::string, std::string> aliases;
    // 别名文件路径
    std::string aliasFilePath;
};
