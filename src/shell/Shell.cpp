#include "shell/Shell.h"

#include "commands/BuiltInCommands.h"
#include "shell/CommandParser.h"
#include "shell/ShellInputParser.h"
#include "utils/ConsoleStyle.h"
#include "utils/EnvUtils.h"
#include "utils/GitUtils.h"
#include "utils/PathUtils.h"
#include "utils/StringUtils.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {
std::string quoteIfNeeded(const std::string& value) {
    if (value.find_first_of(" \t") == std::string::npos) {
        return value;
    }
    return "\"" + value + "\"";
}

std::string firstToken(const std::string& input, size_t& endPos) {
    /* 
    firstToken 获取第一个令牌
    */
    /* 
    输入："ls -l /home"
    输出："ls"
    */
    size_t index = 0;
    while (index < input.size() && std::isspace(static_cast<unsigned char>(input[index]))) {
        ++index;
    }

    if (index >= input.size()) {
        endPos = input.size();
        return "";
    }

    if (input[index] == '"') {
        size_t start = ++index;
        while (index < input.size() && input[index] != '"') {
            ++index;
        }
        endPos = index < input.size() ? index + 1 : index;
        return input.substr(start, index - start);
    }

    size_t start = index;
    while (index < input.size() && !std::isspace(static_cast<unsigned char>(input[index]))) {
        ++index;
    }
    endPos = index;
    return input.substr(start, index - start);
}
}

Shell::Shell() : aliasStore_(".winshellx_aliases"), historyStore_(".winshellx_history") {
    context_.aliasFilePath = aliasStore_.filePath();
    context_.aliases = aliasStore_.load();
    context_.historyFilePath = historyStore_.filePath();
    context_.history = historyStore_.load();
    context_.jobManager = &jobManager_;
    context_.executor = this;
    registerBuiltInCommands(registry_);
}

void Shell::run() {
    ConsoleStyle::writeSuccess("WinShellX started. Type help for commands.\n");
    ConsoleStyle::writeInfo("History loaded from " + context_.historyFilePath +
                            ". Aliases loaded from " + context_.aliasFilePath + ".\n");
    ConsoleStyle::writeInfo("Tab completes history/commands/paths, Up/Down browses history, F7 chooses recent history.\n");
    
    //循环显示当前目录和提示符，读取用户输入，执行命令
    while (context_.running) {
        jobManager_.reapFinished();
        std::string dir = getCurrentDirectoryText();
        std::string branch = currentGitBranch(dir);
        std::string prompt = branch.empty() ? (dir + ">") : ("(" + branch + ")" + dir + ">");

        //readline 读取用户输入（使用while循环，监听用户的输入）
        std::string input = lineEditor_.readLine(prompt, context_.history, registry_.names());
        if (!std::cin && input.empty()) {
            break;
        }
        //trim 去除用户输入的空格
        input = trim(input);
        if (input.empty()) {
            continue;
        }

        context_.history.push_back(input);
        //executeInput 执行命令
        executeInput(expandEnvironmentVariables(expandAlias(input)));
    }

    aliasStore_.save(context_.aliases);
    historyStore_.save(context_.history);
}

bool Shell::executeScriptLine(const std::string& input) {
    std::string line = trim(input);
    if (line.empty()) {
        return true;
    }
    return executeInput(expandEnvironmentVariables(expandAlias(line)));
}

bool Shell::executeInput(const std::string& input) {
    ShellInputPlan plan = parseShellInput(input);

    //如果管道为空，则返回false,一般用户输入的至少有一个命令
    if (plan.pipeline.empty()) {
        return false;
    }

    // 后台作业可以直接继承输出重定向；多段管道涉及多个进程生命周期，暂不纳入作业表。
    if (plan.background && plan.pipeline.size() > 1) {
        ConsoleStyle::writeError("Background mode does not support pipes yet.\n");
        return false;
    }

    std::string pipeText;
    const std::string* stdinPtr = nullptr;

    for (size_t i = 0; i < plan.pipeline.size(); ++i) {
        //如果当前命令不是最后一个命令，则执行当前命令并捕获输出
        const bool isLast = i + 1 == plan.pipeline.size();
        if (!isLast) {
            if (!executeSingle(plan.pipeline[i], stdinPtr, &pipeText, "")) {
                return false;
            }
            stdinPtr = &pipeText;
            continue;
        }

        return executeSingle(plan.pipeline[i], stdinPtr, nullptr, plan.outputFile, plan.background);
    }

    return true;
}

bool Shell::executeSingle(
    const std::string& input,
    const std::string* stdinText,
    std::string* capturedOutput,
    const std::string& outputFilePath,
    bool background) {
    ParsedCommand command = parseCommand(input);
    //find 查找命令
    const ICommand* cmd = registry_.find(command.name);
    
    //如果命令不存在，则运行外部命令
    if (!cmd) {
        ExternalRunOptions options;
        options.stdinText = stdinText;//标准输入
        options.capturedOutput = capturedOutput;//标准输出
        options.outputFilePath = outputFilePath;//输出文件
        options.waitForExit = !background;
        options.jobManager = context_.jobManager;
        if (background && !outputFilePath.empty()) {
            options.displayCommandLine = input + " > " + quoteIfNeeded(outputFilePath);
        }
        return externalCommandRunner_.run(input, options);//运行外部命令
    }

    //如果后台执行，则返回false
    if (background) {
        ConsoleStyle::writeError("Background mode is only supported for external commands.\n");
        return false;
    }

    //如果标准输入不为空，则返回false,因为执行到这里已经是内部命令了，外部命令前面能给标准输入
    if (stdinText != nullptr && !stdinText->empty()) {
        ConsoleStyle::writeError(
            "Internal command '" + command.name + "' does not read piped input. "
            "Use an external filter such as find, e.g. tasklist | find \"chrome\".\n");
    }

    std::streambuf* oldCout = nullptr;
    std::ofstream outputFile;
    std::ostringstream captured;

    //如果标准输出不为空，则捕获标准输出
    if (capturedOutput) {
        //捕获标准输出
        oldCout = std::cout.rdbuf(captured.rdbuf());
    } else if (!outputFilePath.empty()) {
        //如果输出文件不为空，则打开输出文件
        outputFile.open(outputFilePath, std::ios::trunc);
        if (!outputFile) {
            ConsoleStyle::writeError("Could not open redirect file: " + outputFilePath + "\n");
            return false;
        }
        oldCout = std::cout.rdbuf(outputFile.rdbuf());
    }

    //执行命令
    cmd->execute(context_, command.args);

    //恢复标准输出
    if (oldCout) {
        std::cout.rdbuf(oldCout);
    }
    //如果捕获标准输出，则将标准输出赋值给capturedOutput
    if (capturedOutput) {
        *capturedOutput = captured.str();
    }

    return true;
}

std::string Shell::expandAlias(const std::string& input) const {
    size_t endPos = 0;
    std::string token = firstToken(input, endPos);
    if (token.empty() || token == "alias" || token == "unalias") {
        return input;
    }

    auto it = context_.aliases.find(token);
    if (it == context_.aliases.end()) {
        return input;
    }

    return it->second + input.substr(endPos);
}
