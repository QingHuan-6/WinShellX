#pragma once

#include <map>
#include <string>

class AliasStore {
public:
    explicit AliasStore(std::string filePath);

    const std::string& filePath() const;
    std::map<std::string, std::string> load() const;
    void save(const std::map<std::string, std::string>& aliases) const;

private:
    std::string filePath_;
};
