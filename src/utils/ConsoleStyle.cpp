#include "utils/ConsoleStyle.h"

#include <windows.h>

#include <iostream>

namespace {
void writeColored(const std::string& text, WORD color) {
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    bool hasInfo = GetConsoleScreenBufferInfo(output, &info) != 0;
    WORD oldColor = hasInfo ? info.wAttributes : FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

    SetConsoleTextAttribute(output, color);
    std::cout << text;
    SetConsoleTextAttribute(output, oldColor);
}
}

namespace ConsoleStyle {
void writeInfo(const std::string& text) {
    writeColored(text, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

void writeSuccess(const std::string& text) {
    writeColored(text, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

void writeError(const std::string& text) {
    writeColored(text, FOREGROUND_RED | FOREGROUND_INTENSITY);
}

void writePrompt(const std::string& text) {
    writeColored(text, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

void writeDirectoryName(const std::string& text) {
    writeColored(text, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}
}
