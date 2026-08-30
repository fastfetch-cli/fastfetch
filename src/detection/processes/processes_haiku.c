#include "processes.h"

#include <OS.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    system_info info;
    if (get_system_info(&info) != B_OK) {
        return "Error getting system info";
    }

    result->processes = info.used_teams;
    result->threads = info.used_threads;

    if (!options->countKprocs) {
        team_info ti;
        if (get_team_info(B_SYSTEM_TEAM, &ti) == B_OK) {
            result->processes--;
            result->threads -= (uint32_t) ti.thread_count;
        }
    }

    return nullptr;
}
