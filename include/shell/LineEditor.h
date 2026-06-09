#pragma once

#include <string>
#include <vector>

class LineEditor {
public:
    std::string readLine(
        const std::string& prompt,
        const std::vector<std::string>& history,
        const std::vector<std::string>& commandNames) const;

private:
    std::string chooseRecentHistory(const std::string& input, const std::vector<std::string>& history) const;
    void renderInput(const std::string& prompt, const std::string& input, const std::string& hint) const;
};
