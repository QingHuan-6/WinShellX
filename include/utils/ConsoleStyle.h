#pragma once

#include <string>

namespace ConsoleStyle {
void writeInfo(const std::string& text);
void writeSuccess(const std::string& text);
void writeError(const std::string& text);
void writePrompt(const std::string& text);
void writeDirectoryName(const std::string& text);
}
