#include "shell/HistoryStore.h"

#include "utils/StringUtils.h"

#include <fstream>
#include <unordered_set>
#include <utility>

namespace {
constexpr size_t kMaxSavedHistory = 200;
}

HistoryStore::HistoryStore(std::string filePath) : filePath_(std::move(filePath)) {
}

const std::string& HistoryStore::filePath() const {
    return filePath_;
}

std::vector<std::string> HistoryStore::load() const {
    std::ifstream input(filePath_);
    std::vector<std::string> history;
    std::string line;

    while (std::getline(input, line)) {
        line = trim(line);
        if (!line.empty()) {
            history.push_back(line);
        }
    }

    return history;
}

void HistoryStore::save(const std::vector<std::string>& history) const {
    std::ofstream output(filePath_, std::ios::trunc);
    if (!output) {
        return;
    }

    std::vector<std::string> uniqueRecent;
    std::unordered_set<std::string> seen;

    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        std::string command = trim(*it);
        if (command.empty() || seen.count(command) > 0) {
            continue;
        }

        seen.insert(command);
        uniqueRecent.push_back(command);
        if (uniqueRecent.size() >= kMaxSavedHistory) {
            break;
        }
    }

    for (auto it = uniqueRecent.rbegin(); it != uniqueRecent.rend(); ++it) {
        output << *it << "\n";
    }
}
