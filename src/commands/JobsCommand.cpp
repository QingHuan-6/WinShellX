#include "commands/JobsCommand.h"

#include "shell/JobManager.h"
#include "utils/ConsoleStyle.h"

#include <iomanip>
#include <iostream>

std::string JobsCommand::name() const { return "jobs"; }
std::string JobsCommand::usage() const { return "jobs"; }
std::string JobsCommand::description() const { return "List background jobs"; }

void JobsCommand::execute(ShellContext& context, const std::vector<std::string>&) const {
    if (!context.jobManager) {
        ConsoleStyle::writeError("jobs: job manager is not available.\n");
        return;
    }

    std::vector<JobInfo> jobs = context.jobManager->list();
    if (jobs.empty()) {
        ConsoleStyle::writeInfo("No background jobs.\n");
        return;
    }

    std::cout << std::left << std::setw(8) << "Job"
              << std::setw(10) << "PID"
              << std::setw(10) << "Status"
              << "Command\n";
    std::cout << std::string(64, '-') << "\n";

    for (const JobInfo& job : jobs) {
        std::string status = job.status == JobStatus::Running
                                 ? "Running"
                                 : ("Done(" + std::to_string(job.exitCode) + ")");
        std::cout << std::left << std::setw(8) << ("[" + std::to_string(job.id) + "]")
                  << std::setw(10) << job.pid
                  << std::setw(10) << status
                  << job.commandLine << "\n";
    }
}
