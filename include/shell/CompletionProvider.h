#pragma once

#include <string>
#include <vector>

class CompletionProvider {
public:
    CompletionProvider() = default;
    explicit CompletionProvider(std::vector<std::string> commandNames);

    std::string complete(const std::string& input, const std::vector<std::string>& history) const;

private:
    std::vector<std::string> commandNames_;

    std::string completeCommandName(const std::string& input) const;
    std::string completePath(const std::string& input) const;
};
