#pragma once

#include <windows.h>

#include <string>
#include <vector>

enum class JobStatus {
    Running,
    Done
};

struct JobInfo {
    int id = 0;
    DWORD pid = 0;
    std::string commandLine;
    JobStatus status = JobStatus::Running;
    DWORD exitCode = STILL_ACTIVE;
};

class JobManager {
public:
    JobManager() = default;
    ~JobManager();

    JobManager(const JobManager&) = delete;
    JobManager& operator=(const JobManager&) = delete;

    int add(HANDLE processHandle, DWORD pid, const std::string& commandLine);
    std::vector<JobInfo> list();
    bool waitForeground(int id);
    bool terminate(int id);
    void reapFinished();

private:
    struct Job {
        int id = 0;
        DWORD pid = 0;
        HANDLE processHandle = nullptr;
        std::string commandLine;
        JobStatus status = JobStatus::Running;
        DWORD exitCode = STILL_ACTIVE;
    };

    Job* findJob(int id);
    void closeJobHandle(Job& job);

    int nextId_ = 1;
    std::vector<Job> jobs_;
};
