#pragma once

#include <string>

std::string getCurrentDirectoryText();
std::string buildSearchPattern(const std::string& path);
void printDiskSpace(const std::string& path);
