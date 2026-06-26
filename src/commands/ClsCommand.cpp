#include "commands/ClsCommand.h"

#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <windows.h>

std::string ClsCommand::name() const { return "cls"; }
std::string ClsCommand::usage() const { return "cls"; }
std::string ClsCommand::description() const { return "Clear console screen"; }

void ClsCommand::execute(ShellContext&, const std::vector<std::string>&) const {
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (outputHandle == INVALID_HANDLE_VALUE) {
        ConsoleStyle::writeError("cls failed: invalid console handle\n");
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    if (!GetConsoleScreenBufferInfo(outputHandle, &bufferInfo)) {
        ConsoleStyle::writeError("cls failed: " + getLastErrorMessage() + "\n");
        return;
    }

    DWORD cellCount = static_cast<DWORD>(bufferInfo.dwSize.X) * static_cast<DWORD>(bufferInfo.dwSize.Y);
    COORD home = {0, 0};
    DWORD cellsWritten = 0;

    if (!FillConsoleOutputCharacterA(outputHandle, ' ', cellCount, home, &cellsWritten)) {
        ConsoleStyle::writeError("cls failed: " + getLastErrorMessage() + "\n");
        return;
    }

    if (!FillConsoleOutputAttribute(outputHandle, bufferInfo.wAttributes, cellCount, home, &cellsWritten)) {
        ConsoleStyle::writeError("cls failed: " + getLastErrorMessage() + "\n");
        return;
    }

    SetConsoleCursorPosition(outputHandle, home);
}
