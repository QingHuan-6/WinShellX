#include "shell/LineEditor.h"

#include "shell/CompletionProvider.h"

#include <windows.h>

#include <algorithm>
#include <conio.h>
#include <iostream>

namespace {
constexpr WORD kEnterKey = VK_RETURN;
constexpr WORD kBackspaceKey = VK_BACK;
constexpr WORD kTabKey = VK_TAB;
constexpr WORD kF7Key = VK_F7;
constexpr size_t kMaxChoices = 9;

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

void setColor(HANDLE console, WORD attributes) {
    SetConsoleTextAttribute(console, attributes);
}
}

std::string LineEditor::readLine(
    const std::string& prompt,
    const std::vector<std::string>& history,
    const std::vector<std::string>& commandNames) const {
    HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD oldMode = 0;
    bool hasConsoleMode = GetConsoleMode(inputHandle, &oldMode) != 0;

    if (!hasConsoleMode) {
        std::string input;
        std::getline(std::cin, input);
        return input;
    }

    DWORD rawMode = oldMode;
    rawMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(inputHandle, rawMode);

    CompletionProvider completionProvider(commandNames);
    std::string input;
    renderInput(prompt, input, completionProvider.complete(input, history));

    while (true) {
        INPUT_RECORD record;
        DWORD recordsRead = 0;
        if (!ReadConsoleInputA(inputHandle, &record, 1, &recordsRead)) {
            SetConsoleMode(inputHandle, oldMode);
            return input;
        }

        if (record.EventType != KEY_EVENT || !record.Event.KeyEvent.bKeyDown) {
            continue;
        }

        KEY_EVENT_RECORD key = record.Event.KeyEvent;

        if (key.wVirtualKeyCode == kEnterKey) {
            std::cout << "\n";
            SetConsoleMode(inputHandle, oldMode);
            return input;
        }

        if (key.wVirtualKeyCode == kBackspaceKey) {
            if (!input.empty()) {
                input.pop_back();
            }
            renderInput(prompt, input, completionProvider.complete(input, history));
            continue;
        }

        if (key.wVirtualKeyCode == kTabKey) {
            std::string hint = completionProvider.complete(input, history);
            if (!hint.empty()) {
                input = hint;
                renderInput(prompt, input, "");
            }
            continue;
        }

        if (key.wVirtualKeyCode == kF7Key) {
            std::string selected = chooseRecentHistory(input, history);
            if (!selected.empty()) {
                input = selected;
            }
            renderInput(prompt, input, completionProvider.complete(input, history));
            continue;
        }

        char ch = key.uChar.AsciiChar;
        if (ch >= 32 && ch <= 126) {
            input.push_back(ch);
            renderInput(prompt, input, completionProvider.complete(input, history));
        }
    }
}

std::string LineEditor::chooseRecentHistory(const std::string& input, const std::vector<std::string>& history) const {
    std::vector<std::string> matches;

    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if ((input.empty() || startsWith(*it, input)) &&
            std::find(matches.begin(), matches.end(), *it) == matches.end()) {
            matches.push_back(*it);
        }

        if (matches.size() >= kMaxChoices) {
            break;
        }
    }

    std::cout << "\n";
    if (matches.empty()) {
        std::cout << "No matching history.\n";
        return "";
    }

    std::cout << "Recent history:\n";
    for (size_t i = 0; i < matches.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << matches[i] << "\n";
    }
    std::cout << "Choose 1-" << matches.size() << ", or 0 to cancel: ";

    int key = _getch();
    std::cout << static_cast<char>(key) << "\n";
    int choice = key - '0';

    if (choice <= 0 || static_cast<size_t>(choice) > matches.size()) {
        return "";
    }

    return matches[static_cast<size_t>(choice - 1)];
}

void LineEditor::renderInput(const std::string& prompt, const std::string& input, const std::string& hint) const {
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    WORD oldAttributes = 7;
    if (GetConsoleScreenBufferInfo(outputHandle, &info)) {
        oldAttributes = info.wAttributes;
    }

    std::cout << "\r" << std::string(120, ' ') << "\r";
    std::cout << prompt << input;

    if (!hint.empty() && hint.size() > input.size()) {
        setColor(outputHandle, FOREGROUND_INTENSITY);
        std::cout << hint.substr(input.size());
        setColor(outputHandle, oldAttributes);
        std::cout << "\r" << prompt << input;
    }

    std::cout.flush();
}
