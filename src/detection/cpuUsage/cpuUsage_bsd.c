#include "cpuUsage.h"

#include <sys/types.h>
#include <sys/user.h>
#include <sys/sysctl.h>

const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll)
{
    // interrupt processing, user processes, system processing, lock spinning, and idling
    uint64_t cpTime[5];
    size_t neededLength = sizeof(cpTime);
    if(sysctlbyname("kern.cp_time", cpTime, &neededLength, NULL, 0) != 0)
        return "sysctlbyname(kern.cp_time) failed";

    *inUseAll = cpTime[0] + cpTime[1] + cpTime[2] + cpTime[3];
    *totalAll = *inUseAll + cpTime[4];

    return NULL;
}
