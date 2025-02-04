#include "processes.h"

#include <OS.h>

const char* ffDetectProcesses(uint32_t* result)
{
    system_info info;
    if (get_system_info(&info) != B_OK)
        return "Error getting system info";

    *result = info.used_teams;

    return NULL;
}
