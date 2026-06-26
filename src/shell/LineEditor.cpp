#include "shell/LineEditor.h"

#include "shell/CompletionProvider.h"
#include "utils/ConsoleStyle.h"

#include <windows.h>

#include <algorithm>
#include <conio.h>
#include <iostream>

namespace {
constexpr WORD kEnterKey = VK_RETURN;
constexpr WORD kBackspaceKey = VK_BACK;
constexpr WORD kDeleteKey = VK_DELETE;
constexpr WORD kTabKey = VK_TAB;
constexpr WORD kF7Key = VK_F7;
constexpr WORD kUpKey = VK_UP;
constexpr WORD kDownKey = VK_DOWN;
constexpr WORD kLeftKey = VK_LEFT;
constexpr WORD kRightKey = VK_RIGHT;
constexpr WORD kHomeKey = VK_HOME;
constexpr WORD kEndKey = VK_END;
constexpr size_t kMaxChoices = 9;

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

void setColor(HANDLE console, WORD attributes) {
    SetConsoleTextAttribute(console, attributes);
}

bool stdinIsInteractive() {
    HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (inputHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = 0;
    if (GetConsoleMode(inputHandle, &mode) == 0) {
        return false;
    }

    return GetFileType(inputHandle) == FILE_TYPE_CHAR;
}

std::string completionHint(
    const CompletionProvider& provider,
    const std::string& input,
    const std::vector<std::string>& history,
    size_t cursorPos) {
    if (cursorPos != input.size()) {
        return "";
    }
    return provider.complete(input, history);
}
}

std::string LineEditor::readLine(
    const std::string& prompt,
    const std::vector<std::string>& history,
    const std::vector<std::string>& commandNames) const {
    HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD oldMode = 0;
    bool hasConsoleMode = GetConsoleMode(inputHandle, &oldMode) != 0;

    if (!hasConsoleMode || !stdinIsInteractive()) {
        std::string input;
        std::getline(std::cin, input);
        ConsoleStyle::writePrompt(prompt);
        std::cout << input << "\n";
        return input;
    }

    DWORD rawMode = oldMode;
    rawMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(inputHandle, rawMode);

    CompletionProvider completionProvider(commandNames);
    std::string input;
    std::string draftInput;
    size_t cursorPos = 0;
    int historyIndex = -1;
    renderInput(prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);

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

        if (key.wVirtualKeyCode == kLeftKey) {
            if (cursorPos > 0) {
                --cursorPos;
            }
            renderInput(
                prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            continue;
        }

        if (key.wVirtualKeyCode == kRightKey) {
            if (cursorPos < input.size()) {
                ++cursorPos;
            }
            renderInput(
                prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            continue;
        }

        if (key.wVirtualKeyCode == kHomeKey) {
            cursorPos = 0;
            renderInput(
                prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            continue;
        }

        if (key.wVirtualKeyCode == kEndKey) {
            cursorPos = input.size();
            renderInput(
                prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            continue;
        }

        if (key.wVirtualKeyCode == kBackspaceKey) {
            if (cursorPos > 0) {
                input.erase(cursorPos - 1, 1);
                --cursorPos;
            }
            historyIndex = -1;
            renderInput(
                prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            continue;
        }

        if (key.wVirtualKeyCode == kDeleteKey) {
            if (cursorPos < input.size()) {
                input.erase(cursorPos, 1);
            }
            historyIndex = -1;
            renderInput(
                prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            continue;
        }

        if (key.wVirtualKeyCode == kTabKey) {
            std::string hint = completionProvider.complete(input, history);
            if (!hint.empty()) {
                input = hint;
                cursorPos = input.size();
                historyIndex = -1;
                renderInput(prompt, input, "", cursorPos);
            }
            continue;
        }

        if (key.wVirtualKeyCode == kUpKey) {
            if (!history.empty()) {
                if (historyIndex == -1) {
                    draftInput = input;
                    historyIndex = static_cast<int>(history.size());
                }
                if (historyIndex > 0) {
                    --historyIndex;
                    input = history[static_cast<size_t>(historyIndex)];
                    cursorPos = input.size();
                }
                renderInput(
                    prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            }
            continue;
        }

        if (key.wVirtualKeyCode == kDownKey) {
            if (historyIndex != -1) {
                ++historyIndex;
                if (historyIndex >= static_cast<int>(history.size())) {
                    historyIndex = -1;
                    input = draftInput;
                } else {
                    input = history[static_cast<size_t>(historyIndex)];
                }
                cursorPos = input.size();
                renderInput(
                    prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            }
            continue;
        }

        if (key.wVirtualKeyCode == kF7Key) {
            std::string selected = chooseRecentHistory(input, history);
            if (!selected.empty()) {
                input = selected;
                cursorPos = input.size();
                historyIndex = -1;
            }
            renderInput(
                prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
            continue;
        }

        char ch = key.uChar.AsciiChar;
        if (ch >= 32 && ch <= 126) {
            input.insert(cursorPos, 1, ch);
            ++cursorPos;
            historyIndex = -1;
            renderInput(
                prompt, input, completionHint(completionProvider, input, history, cursorPos), cursorPos);
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
    std::cout << "Choose 1-" << matches.size() << ", 0 to cancel: ";

    int key = _getch();
    std::cout << static_cast<char>(key) << "\n";
    int choice = key - '0';

    if (choice <= 0 || static_cast<size_t>(choice) > matches.size()) {
        return "";
    }

    return matches[static_cast<size_t>(choice - 1)];
}

void LineEditor::renderInput(
    const std::string& prompt,
    const std::string& input,
    const std::string& hint,
    size_t cursorPos) const {
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    WORD oldAttributes = 7;
    if (!GetConsoleScreenBufferInfo(outputHandle, &info)) {
        std::cout << "\r" << prompt << input << std::flush;
        return;
    }

    oldAttributes = info.wAttributes;
    const SHORT row = info.dwCursorPosition.Y;

    std::cout << "\r" << std::string(120, ' ') << "\r";
    ConsoleStyle::writePrompt(prompt);

    CONSOLE_SCREEN_BUFFER_INFO afterPrompt;
    if (!GetConsoleScreenBufferInfo(outputHandle, &afterPrompt)) {
        std::cout << input << std::flush;
        return;
    }

    const SHORT inputStartCol = afterPrompt.dwCursorPosition.X;
    std::cout << input;

    if (!hint.empty() && hint.size() > input.size()) {
        setColor(outputHandle, FOREGROUND_INTENSITY);
        std::cout << hint.substr(input.size());
        setColor(outputHandle, oldAttributes);
    }

    const size_t safeCursor = (std::min)(cursorPos, input.size());
    COORD pos = {static_cast<SHORT>(inputStartCol + static_cast<SHORT>(safeCursor)), row};
    SetConsoleCursorPosition(outputHandle, pos);
    std::cout.flush();
}
