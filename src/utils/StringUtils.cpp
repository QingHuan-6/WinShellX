#include "utils/StringUtils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

std::string trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });

    if (first == value.end()) {
        return "";
    }

    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();

    return std::string(first, last);
}

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::vector<std::string> splitArguments(const std::string& line) {
    std::vector<std::string> result;
    std::string current;
    bool inQuotes = false;

    for (char ch : line) {
        if (ch == '"') {
            inQuotes = !inQuotes;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) && !inQuotes) {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty()) {
        result.push_back(current);
    }

    return result;
}

std::string joinArgs(const std::vector<std::string>& args) {
    std::ostringstream joined;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            joined << ' ';
        }
        joined << args[i];
    }
    return joined.str();
}
