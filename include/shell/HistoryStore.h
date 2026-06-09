#pragma once

#include <string>
#include <vector>

class HistoryStore {
public:
    explicit HistoryStore(std::string filePath);

    const std::string& filePath() const;
    std::vector<std::string> load() const;
    void save(const std::vector<std::string>& history) const;

private:
    std::string filePath_;
};
