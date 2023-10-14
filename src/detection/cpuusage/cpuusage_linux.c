#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"
#include "common/io/io.h"

#include <stdio.h>
#include <inttypes.h>

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    FF_AUTO_CLOSE_FILE FILE* procStat = fopen("/proc/stat", "r");
    if(procStat == NULL)
    {
        #ifdef __ANDROID__
        return "Accessing \"/proc/stat\" is restricted on Android O+";
        #else
        return "fopen(\"""/proc/stat\", \"r\") == NULL";
        #endif
    }
    // Skip first line
    if (fscanf(procStat, "cpu%*[^\n]\n") < 0)
        return "fscanf() first line failed";

    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0;
    while (fscanf(procStat, "cpu%*d%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%*[^\n]\n", &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7)
    {
        uint64_t inUse = user + nice + system;
        uint64_t total = inUse + idle + iowait + irq + softirq;

        FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
        *info = (FFCpuUsageInfo) {
            .inUseAll = (uint64_t)inUse,
            .totalAll = (uint64_t)total,
        };
    }

    return NULL;
}
