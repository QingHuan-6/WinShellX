#include "shell/ExternalCommandRunner.h"

#include "shell/CommandResolver.h"
#include "shell/ConsoleControl.h"
#include "shell/JobManager.h"
#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <windows.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {
std::string quote(const std::string& value) {
    return "\"" + value + "\"";
}

std::string getEnvironmentValue(const char* name, const std::string& fallback = "") {
    DWORD size = GetEnvironmentVariableA(name, nullptr, 0);
    if (size == 0) {
        return fallback;
    }

    std::vector<char> buffer(size);
    DWORD written = GetEnvironmentVariableA(name, buffer.data(), size);
    if (written == 0 || written >= size) {
        return fallback;
    }

    return std::string(buffer.data(), written);
}

bool fileExists(const std::string& path) {
    DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
           (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

std::string joinPath(const std::string& directory, const std::string& fileName) {
    if (directory.empty() || directory == ".") {
        return fileName;
    }

    char last = directory.back();
    if (last == '\\' || last == '/') {
        return directory + fileName;
    }

    return directory + "\\" + fileName;
}

std::string cmdExecutablePath() {
    std::string comspec = getEnvironmentValue("ComSpec");
    if (!comspec.empty() && fileExists(comspec)) {
        return comspec;
    }

    char systemDirectory[MAX_PATH];
    if (GetSystemDirectoryA(systemDirectory, MAX_PATH) > 0) {
        std::string candidate = joinPath(systemDirectory, "cmd.exe");
        if (fileExists(candidate)) {
            return candidate;
        }
    }

    return "cmd.exe";
}

std::string buildProcessCommandLine(const ResolvedCommand& command) {
    if (command.batchFile) {
        std::string line = quote(cmdExecutablePath()) + " /d /s /c \"";
        line += quote(command.executablePath);
        if (!command.arguments.empty()) {
            line += " " + command.arguments;
        }
        line += "\"";
        return line;
    }

    std::string line = quote(command.executablePath);
    if (!command.arguments.empty()) {
        line += " " + command.arguments;
    }
    return line;
}

std::string applicationNameFor(const ResolvedCommand& command) {
    return command.batchFile ? cmdExecutablePath() : command.executablePath;
}

void closeIfValid(HANDLE& handle) {
    if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        handle = nullptr;
    }
}

void appendFromPipe(HANDLE readHandle, std::string* output) {
    char buffer[4096];
    DWORD bytesRead = 0;
    //读取管道数据
    while (ReadFile(readHandle, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0) {
        if (output) {
            //将数据添加到输出中
            output->append(buffer, buffer + bytesRead);
        }
    }
}
}

bool ExternalCommandRunner::run(const std::string& commandLine, const ExternalRunOptions& options) const {
    //如果命令行是空，则返回false
    if (commandLine.empty()) {
        return false;
    }
    //解析命令行
    ResolvedCommand resolved;
    if (!resolveCommand(commandLine, resolved)) {
        ConsoleStyle::writeError("'" + commandNameFromLine(commandLine)
                  + "' is not recognized as an internal or external command.\n");
        return false;
    }
    //构建进程命令行
    std::string processCommandLine = buildProcessCommandLine(resolved);
    //获取应用程序名称
    std::string applicationName = applicationNameFor(resolved);
    //构建可变命令
    std::vector<char> mutableCommand(processCommandLine.begin(), processCommandLine.end());
    mutableCommand.push_back('\0');

    //创建安全属性
    SECURITY_ATTRIBUTES securityAttributes;
    ZeroMemory(&securityAttributes, sizeof(securityAttributes));
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;//继承句柄

    //创建句柄
    HANDLE stdinRead = nullptr;
    HANDLE stdinWrite = nullptr;
    HANDLE stdoutRead = nullptr;
    HANDLE stdoutWrite = nullptr;
    HANDLE outputFile = nullptr;

    bool redirectInput = options.stdinText != nullptr;
    bool captureOutput = options.capturedOutput != nullptr;//如果捕获输出不为空指针，代表父进程要捕获输出
    bool redirectOutputFile = !options.outputFilePath.empty();//重定向输出文件

    //如果重定向输入(如sort < input.txt)，则创建匿名管道
    if (redirectInput) {
        // 管道左侧输出作为右侧标准输入时，用匿名管道把文本写给子进程 stdin。
        if (!CreatePipe(&stdinRead, &stdinWrite, &securityAttributes, 0)) {
            ConsoleStyle::writeError("CreatePipe failed: " + getLastErrorMessage() + "\n");
            return false;
        }
        SetHandleInformation(stdinWrite, HANDLE_FLAG_INHERIT, 0);
    }

    //如果捕获输出，则创建匿名管道
    if (captureOutput) {
        // 管道或内部捕获需要拦截子进程 stdout/stderr，读端留给父进程读取。
        if (!CreatePipe(&stdoutRead, &stdoutWrite, &securityAttributes, 0)) {
            ConsoleStyle::writeError("CreatePipe failed: " + getLastErrorMessage() + "\n");
            closeIfValid(stdinRead);
            closeIfValid(stdinWrite);
            return false;
        }
        SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0);
    } 
    //如果重定向输出文件，则创建文件
    else if (redirectOutputFile) {
        // 支持外部命令 > file；后台作业也可以继承这个输出文件句柄。
        outputFile = CreateFileA(
            options.outputFilePath.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ,
            &securityAttributes,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (outputFile == INVALID_HANDLE_VALUE) {
            ConsoleStyle::writeError("Could not open redirect file: " + getLastErrorMessage() + "\n");
            closeIfValid(stdinRead);
            closeIfValid(stdinWrite);
            return false;
        }
    }
    //创建启动信息
    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInfo, sizeof(processInfo));
    startupInfo.cb = sizeof(startupInfo);
    if (redirectInput || captureOutput || redirectOutputFile) {
        // STARTF_USESTDHANDLES 让 CreateProcess 使用我们准备好的 stdin/stdout/stderr。
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;
        //如果重定向输入，则使用stdinRead,否则使用标准输入
        startupInfo.hStdInput = redirectInput ? stdinRead : GetStdHandle(STD_INPUT_HANDLE);
        //如果捕获输出，则使用stdoutWrite
        startupInfo.hStdOutput = captureOutput ? stdoutWrite
                            //如果重定向输出文件，则使用outputFile
                            : redirectOutputFile ? outputFile
                                                 : GetStdHandle(STD_OUTPUT_HANDLE);
        startupInfo.hStdError = startupInfo.hStdOutput;
    }

    //创建进程，运行外部命令（其中已经包含重定向输入、捕获输出、重定向输出文件）
    BOOL created = CreateProcessA(
        applicationName.c_str(),
        mutableCommand.data(),
        nullptr,
        nullptr,
        (redirectInput || captureOutput || redirectOutputFile) ? TRUE : FALSE,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo);

    //如果创建进程失败，则关闭句柄
    if (!created) {
        ConsoleStyle::writeError("Failed to start external program: "
                  + resolved.executablePath + "\n");
        ConsoleStyle::writeError("CreateProcess failed: " + getLastErrorMessage() + "\n");
        closeIfValid(stdinRead);
        closeIfValid(stdinWrite);
        closeIfValid(stdoutRead);
        closeIfValid(stdoutWrite);
        closeIfValid(outputFile);
        return false;
    }

    closeIfValid(stdinRead);
    closeIfValid(stdoutWrite);
    closeIfValid(outputFile);

    //创建线程
    std::thread reader;
    //如果捕获输出，则这样创建线程
    if (captureOutput) {
        //创建线程，参数为stdoutRead和options.capturedOutput
        //作用是读取管道数据，并添加到options.capturedOutput中
        reader = std::thread(appendFromPipe, stdoutRead, options.capturedOutput);
    }

    //如果重定向输入，则写入数据
    if (redirectInput && options.stdinText) {
        DWORD bytesWritten = 0;
        WriteFile(stdinWrite, options.stdinText->data(), static_cast<DWORD>(options.stdinText->size()), &bytesWritten, nullptr);
        closeIfValid(stdinWrite);
    }

    if (options.waitForExit) {
        // 前台等待：登记子进程句柄并用短超时轮询，以便 Ctrl+C 中断时能及时收尾。
        ConsoleControl::setForegroundProcess(processInfo.hProcess);
        ConsoleControl::resetInterrupt();
        for (;;) {
            if (WaitForSingleObject(processInfo.hProcess, 100) == WAIT_OBJECT_0) {
                break;
            }
            if (ConsoleControl::interrupted()) {
                // Ctrl+C 已同时投递给子进程，给它一点时间自行退出；仍未退出则强杀。
                if (WaitForSingleObject(processInfo.hProcess, 500) != WAIT_OBJECT_0) {
                    TerminateProcess(processInfo.hProcess, 1);
                    WaitForSingleObject(processInfo.hProcess, 500);
                }
                std::cout << "^C\n";
                break;
            }
        }
        ConsoleControl::clearForegroundProcess();
        if (reader.joinable()) {
            reader.join();
        }
    } else {
        if (options.jobManager) {
            // 后台模式下进程句柄移交给 JobManager，不能在这里关闭，否则 jobs/fg/killjob 失效。
            int jobId = options.jobManager->add(
                processInfo.hProcess,
                processInfo.dwProcessId,
                options.displayCommandLine.empty() ? commandLine : options.displayCommandLine);
            ConsoleStyle::writeSuccess("Started job [" + std::to_string(jobId) +
                                       "] PID " + std::to_string(processInfo.dwProcessId) +
                                       ".\n");
            processInfo.hProcess = nullptr;
        } else {
            ConsoleStyle::writeSuccess("Started background process, PID: " +
                                       std::to_string(processInfo.dwProcessId) + "\n");
        }
        if (reader.joinable()) {
            reader.detach();
        }
    }

    closeIfValid(stdoutRead);
    closeIfValid(stdinWrite);
    CloseHandle(processInfo.hThread);
    closeIfValid(processInfo.hProcess);
    return true;
}
