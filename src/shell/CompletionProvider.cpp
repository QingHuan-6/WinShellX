#include "shell/CompletionProvider.h"

#include "utils/StringUtils.h"

#include <windows.h>

namespace {
bool startsWithIgnoreCase(const std::string& value, const std::string& prefix) {
    //这个函数的目的是判断value是否以prefix开头，不区分大小写
    //将value和prefix转换为小写
    std::string lowerValue = toLower(value);
    std::string lowerPrefix = toLower(prefix);
    //比较value和prefix的大小，如果value大于等于prefix，则返回true
    return lowerValue.size() >= lowerPrefix.size() &&
           lowerValue.compare(0, lowerPrefix.size(), lowerPrefix) == 0;
}

std::string longestCommonPrefix(const std::vector<std::string>& values) {
    //如果values为空，则返回空字符串
    if (values.empty()) {
        return "";
    }
    //获取values的第一个元素
    std::string prefix = values.front();
    //遍历values
    for (const std::string& value : values) {
        //获取value的第一个字符
        size_t index = 0;
        //比较prefix和value的大小，如果prefix大于等于value，则返回true
        while (index < prefix.size() &&
               index < value.size() &&
               std::tolower(static_cast<unsigned char>(prefix[index])) ==
                   std::tolower(static_cast<unsigned char>(value[index]))) {
            ++index;
        }
        prefix.resize(index);
    }
    //返回最长的公共前缀
    return prefix;
}

size_t lastSeparatorPosition(const std::string& value) {
    size_t slash = value.find_last_of("\\/");
    return slash == std::string::npos ? 0 : slash + 1;
}

std::string quoteIfNeeded(const std::string& value) {
    if (value.find(' ') == std::string::npos) {
        return value;
    }
    return "\"" + value + "\"";
}
}

CompletionProvider::CompletionProvider(std::vector<std::string> commandNames)
    : commandNames_(std::move(commandNames)) {
}

std::string CompletionProvider::complete(const std::string& input, const std::vector<std::string>& history) const {
    if (!input.empty()) {
        //从历史记录中查找最长的前缀匹配
        for (auto it = history.rbegin(); it != history.rend(); ++it) {
            if (*it != input && startsWithIgnoreCase(*it, input)) {
                return *it; 
            }
        }
    }

    //如果input中不包含空格和制表符，则补全命令名
    if (input.find(' ') == std::string::npos && input.find('\t') == std::string::npos) {
        return completeCommandName(input);
    }

    //如果input中包含空格和制表符，则补全路径
    return completePath(input);
}

std::string CompletionProvider::completeCommandName(const std::string& input) const {
    if (input.empty()) {
        return "";
    }

    std::vector<std::string> matches;
    for (const std::string& name : commandNames_) {
        if (startsWithIgnoreCase(name, input)) {
            matches.push_back(name);
        }
    }

    if (matches.empty()) {
        return "";
    }

    std::string common = longestCommonPrefix(matches);
    return common.size() > input.size() ? common : matches.front();
}

std::string CompletionProvider::completePath(const std::string& input) const {
    //如果input中不包含空格，则返回空字符串
    size_t argStart = input.find(' ');
    if (argStart == std::string::npos) {
        return "";
    }

    //获取前缀
    std::string prefix = input.substr(0, argStart + 1);
    //获取路径部分
    std::string pathPart = input.substr(argStart + 1);
    bool quoted = !pathPart.empty() && pathPart.front() == '"';
    if (quoted) {
        pathPart.erase(pathPart.begin());
    }
    //获取路径部分的最后一个分隔符的位置
    size_t nameStart = lastSeparatorPosition(pathPart);
    //获取路径部分的目录部分
    std::string directory = nameStart == 0 ? "." : pathPart.substr(0, nameStart);
    //获取路径部分的文件名部分
    std::string namePrefix = pathPart.substr(nameStart);
    //获取搜索基础路径
    std::string searchBase = directory;
    //如果搜索基础路径为空，则设置为当前目录
    if (searchBase.empty()) {
        searchBase = ".";
    }

    //构建搜索模式
    std::string pattern = searchBase;
    //如果搜索模式不为空，并且搜索模式的最后一个字符不是反斜杠或斜杠，则添加反斜杠
    if (!pattern.empty() && pattern.back() != '\\' && pattern.back() != '/') {
        pattern += "\\";
    }
    //添加通配符
    pattern += "*";

    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA(pattern.c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE) {
        return "";
    }

    //遍历搜索结果
    std::vector<std::string> matches;
    do {
        std::string name = data.cFileName;
        if (name == "." || name == ".." || !startsWithIgnoreCase(name, namePrefix)) {
            continue;
        }

        std::string completedPath = nameStart == 0 ? name : pathPart.substr(0, nameStart) + name;
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            completedPath += "\\";
        }
        matches.push_back(completedPath);
    } while (FindNextFileA(handle, &data));

    FindClose(handle);

    if (matches.empty()) {
        return "";
    }
    //获取最长的公共前缀
    std::string common = longestCommonPrefix(matches);
    std::string completed = common.size() > pathPart.size() ? common : matches.front();
    return prefix + quoteIfNeeded(completed);
}
