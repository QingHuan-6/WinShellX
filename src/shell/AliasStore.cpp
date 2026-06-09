#include "shell/AliasStore.h"

#include "utils/StringUtils.h"

#include <fstream>
#include <utility>

AliasStore::AliasStore(std::string filePath) : filePath_(std::move(filePath)) {
}

const std::string& AliasStore::filePath() const {
    return filePath_;
}

std::map<std::string, std::string> AliasStore::load() const {
    std::ifstream input(filePath_);
    std::map<std::string, std::string> aliases;
    std::string line;

    while (std::getline(input, line)) {
        size_t equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        std::string name = trim(line.substr(0, equals));
        std::string value = trim(line.substr(equals + 1));
        if (!name.empty() && !value.empty()) {
            aliases[name] = value;
        }
    }

    return aliases;
}

void AliasStore::save(const std::map<std::string, std::string>& aliases) const {
    std::ofstream output(filePath_, std::ios::trunc);
    if (!output) {
        return;
    }

    for (const auto& item : aliases) {
        output << item.first << "=" << item.second << "\n";
    }
}
