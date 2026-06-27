#pragma once

#include <string>
#include <vector>

// 解析后的外部命令：可执行文件全路径、剩余参数、是否为批处理文件。
struct ResolvedCommand {
    std::string executablePath;
    std::string arguments;
    bool batchFile = false;
};

// 按 PATH/PATHEXT 规则把一行外部命令解析为可执行文件路径。
// 例如 "notepad a.txt" -> executablePath="C:\Windows\System32\notepad.exe", arguments="a.txt"。
bool resolveCommand(const std::string& commandLine, ResolvedCommand& resolved);

// 查找命令名在所有搜索目录与 PATHEXT 后缀下的全部匹配路径（供 where 命令使用）。
std::vector<std::string> resolveAllExecutablePaths(const std::string& commandName);

// 从命令行中提取首个 token（可执行文件名），用于错误提示。
std::string commandNameFromLine(const std::string& commandLine);
