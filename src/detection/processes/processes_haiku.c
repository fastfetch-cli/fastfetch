#include "processes.h"

#include <OS.h>

const char* ffDetectProcesses(const FFProcessesOptions*, FFProcessesResult* result) {
    system_info info;
    if (get_system_info(&info) != B_OK) {
        return "Error getting system info";
    }

    result->processes = info.used_teams;
    result->threads = info.used_threads;

    return nullptr;
}
