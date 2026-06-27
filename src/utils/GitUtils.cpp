#include "utils/GitUtils.h"

#include "utils/StringUtils.h"

#include <windows.h>

#include <fstream>
#include <string>

namespace {
bool isDirectory(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool isAbsolutePath(const std::string& path) {
    if (path.size() >= 2 && path[1] == ':') {
        return true;
    }
    return !path.empty() && (path[0] == '\\' || path[0] == '/');
}

std::string parentDirectory(const std::string& dir) {
    std::string current = dir;
    while (current.size() > 1 && (current.back() == '\\' || current.back() == '/')) {
        current.pop_back();
    }

    size_t sep = current.find_last_of("\\/");
    if (sep == std::string::npos || sep == 0) {
        return "";
    }

    std::string parent = current.substr(0, sep);
    if (parent.size() == 2 && parent[1] == ':') {
        parent += "\\";
    }
    return parent;
}

std::string readGitHead(const std::string& gitDir) {
    std::ifstream head(gitDir + "\\HEAD");
    std::string line;
    if (!std::getline(head, line)) {
        return "";
    }
    line = trim(line);

    const std::string refPrefix = "ref: refs/heads/";
    if (line.size() > refPrefix.size() && line.compare(0, refPrefix.size(), refPrefix) == 0) {
        return line.substr(refPrefix.size());
    }
    return line.substr(0, 7);
}
}

std::string currentGitBranch(const std::string& directory) {
    std::string current = directory.empty() ? std::string(".") : directory;
    while (current.size() > 1 && (current.back() == '\\' || current.back() == '/')) {
        current.pop_back();
    }

    std::string gitDir;
    while (!current.empty()) {
        std::string candidate = current + "\\.git";
        DWORD attr = GetFileAttributesA(candidate.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES) {
            gitDir = candidate;
            break;
        }
        std::string parent = parentDirectory(current);
        if (parent == current || parent.empty()) {
            break;
        }
        current = parent;
    }

    if (gitDir.empty()) {
        return "";
    }

    // 工作树/子模块情形：.git 是一个文件，内容为 "gitdir: <path>"
    if (!isDirectory(gitDir)) {
        std::ifstream file(gitDir);
        std::string line;
        if (!std::getline(file, line)) {
            return "";
        }
        line = trim(line);
        const std::string prefix = "gitdir:";
        if (line.size() <= prefix.size() || line.compare(0, prefix.size(), prefix) != 0) {
            return "";
        }
        std::string target = trim(line.substr(prefix.size()));
        if (target.empty()) {
            return "";
        }
        gitDir = isAbsolutePath(target) ? target : (current + "\\" + target);
    }

    return readGitHead(gitDir);
}
