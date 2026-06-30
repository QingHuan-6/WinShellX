#pragma once

#include <windows.h>

// 控制台 Ctrl+C/Ctrl+Break 中断管理。
// 目的：让 WinShellX 在收到 Ctrl+C 时不被系统默认处理终止，
// 而是设置中断标志，由 ExternalCommandRunner 据此中断正在运行的前台子进程。
class ConsoleControl {
public:
    // 注册控制台控制处理函数，进程内只需调用一次。
    static void install();

    // 记录/清除当前正在前台等待的子进程句柄。
    static void setForegroundProcess(HANDLE process);
    static void clearForegroundProcess();

    // 中断标志：自上次 reset 以来是否收到过 Ctrl+C。
    static bool interrupted();
    static void resetInterrupt();
};
