#pragma once

#include <string>

// 返回 directory 所在 git 仓库的当前分支名（detached HEAD 时返回短哈希）；
// 不在 git 仓库内时返回空串。
std::string currentGitBranch(const std::string& directory);
