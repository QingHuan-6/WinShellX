#pragma once

#include <string>
#include <vector>

std::string trim(const std::string& value);
std::string toLower(std::string value);
std::vector<std::string> splitArguments(const std::string& line);
std::string joinArgs(const std::vector<std::string>& args);
