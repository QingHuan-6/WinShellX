#include "shell/JobManager.h"

#include "utils/ConsoleStyle.h"
#include "utils/WinApiError.h"

#include <tlhelp32.h>

#include <algorithm>
#include <iostream>
#include <vector>

namespace {
std::vector<DWORD> childProcessesOf(DWORD parentPid) {
    std::vector<DWORD> children;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return children;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        return children;
    }

    do {
        if (entry.th32ParentProcessID == parentPid) {
            children.push_back(entry.th32ProcessID);
        }
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return children;
}

void terminateProcessTree(DWORD rootPid) {
    for (DWORD childPid : childProcessesOf(rootPid)) {
        terminateProcessTree(childPid);
    }

    HANDLE process = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, rootPid);
    if (!process) {
        return;
    }

    TerminateProcess(process, 1);
    WaitForSingleObject(process, 3000);
    CloseHandle(process);
}
}

JobManager::~JobManager() {
    for (Job& job : jobs_) {
        closeJobHandle(job);
    }
}

int JobManager::add(HANDLE processHandle, DWORD pid, const std::string& commandLine) {
    Job job;
    job.id = nextId_++;
    job.pid = pid;
    job.processHandle = processHandle;
    job.commandLine = commandLine;
    jobs_.push_back(job);
    return job.id;
}

std::vector<JobInfo> JobManager::list() {
    reapFinished();

    std::vector<JobInfo> result;
    for (const Job& job : jobs_) {
        JobInfo info;
        info.id = job.id;
        info.pid = job.pid;
        info.commandLine = job.commandLine;
        info.status = job.status;
        info.exitCode = job.exitCode;
        result.push_back(info);
    }

    jobs_.erase(
        std::remove_if(jobs_.begin(), jobs_.end(), [this](Job& job) {
            if (job.status == JobStatus::Done) {
                closeJobHandle(job);
                return true;
            }
            return false;
        }),
        jobs_.end());

    return result;
}

bool JobManager::waitForeground(int id) {
    reapFinished();
    Job* job = findJob(id);
    if (!job) {
        ConsoleStyle::writeError("fg: no such running job: " + std::to_string(id) + "\n");
        return false;
    }

    ConsoleStyle::writeInfo("Foreground job [" + std::to_string(job->id) + "] PID " +
                            std::to_string(job->pid) + ": " + job->commandLine + "\n");
    DWORD waitResult = WaitForSingleObject(job->processHandle, INFINITE);
    if (waitResult == WAIT_FAILED) {
        ConsoleStyle::writeError("fg failed: " + getLastErrorMessage() + "\n");
        return false;
    }

    DWORD exitCode = 0;
    GetExitCodeProcess(job->processHandle, &exitCode);
    ConsoleStyle::writeSuccess("Job [" + std::to_string(job->id) + "] finished, exit code " +
                               std::to_string(exitCode) + ".\n");
    closeJobHandle(*job);
    jobs_.erase(std::remove_if(jobs_.begin(), jobs_.end(), [id](const Job& item) {
                    return item.id == id;
                }),
                jobs_.end());
    return true;
}

bool JobManager::terminate(int id) {
    reapFinished();
    Job* job = findJob(id);
    if (!job) {
        ConsoleStyle::writeError("killjob: no such running job: " + std::to_string(id) + "\n");
        return false;
    }

    terminateProcessTree(job->pid);
    WaitForSingleObject(job->processHandle, 3000);
    ConsoleStyle::writeSuccess("Killed job [" + std::to_string(job->id) + "] PID " +
                               std::to_string(job->pid) + ".\n");
    closeJobHandle(*job);
    jobs_.erase(std::remove_if(jobs_.begin(), jobs_.end(), [id](const Job& item) {
                    return item.id == id;
                }),
                jobs_.end());
    return true;
}

void JobManager::reapFinished() {
    for (Job& job : jobs_) {
        if (job.status == JobStatus::Done || job.processHandle == nullptr) {
            continue;
        }

        DWORD waitResult = WaitForSingleObject(job.processHandle, 0);
        if (waitResult == WAIT_OBJECT_0) {
            DWORD exitCode = 0;
            GetExitCodeProcess(job.processHandle, &exitCode);
            job.exitCode = exitCode;
            job.status = JobStatus::Done;
        }
    }
}

JobManager::Job* JobManager::findJob(int id) {
    for (Job& job : jobs_) {
        if (job.id == id && job.status == JobStatus::Running) {
            return &job;
        }
    }
    return nullptr;
}

void JobManager::closeJobHandle(Job& job) {
    if (job.processHandle != nullptr && job.processHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(job.processHandle);
        job.processHandle = nullptr;
    }
}
