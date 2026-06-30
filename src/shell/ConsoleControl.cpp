#include "shell/ConsoleControl.h"

#include <atomic>

namespace {
std::atomic<HANDLE> g_foregroundProcess{nullptr};
std::atomic<bool> g_interrupted{false};

BOOL WINAPI consoleCtrlHandler(DWORD ctrlType) {
    if (ctrlType == CTRL_C_EVENT || ctrlType == CTRL_BREAK_EVENT) {
        // Ctrl+C 事件会同时投递给挂接到本控制台的每个进程，子进程通常会自行退出。
        // 这里返回 TRUE，阻止系统默认处理把 WinShellX 自身终止掉。
        g_interrupted.store(true);
        return TRUE;
    }
    // CLOSE/LOGOFF/SHUTDOWN 等事件交给默认处理。
    return FALSE;
}
}

void ConsoleControl::install() {
    SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
}

void ConsoleControl::setForegroundProcess(HANDLE process) {
    g_foregroundProcess.store(process);
}

void ConsoleControl::clearForegroundProcess() {
    g_foregroundProcess.store(nullptr);
}

bool ConsoleControl::interrupted() {
    return g_interrupted.load();
}

void ConsoleControl::resetInterrupt() {
    g_interrupted.store(false);
}
