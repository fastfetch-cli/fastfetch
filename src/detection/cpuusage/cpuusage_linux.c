#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"
#include "common/io/io.h"

#include <stdio.h>
#include <inttypes.h>

const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll)
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

    *inUseAll = 0;
    *totalAll = 0;

    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0;
    while (fscanf(procStat, "cpu%*d%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64, &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7)
    {
        uint64_t inUse = user + nice + system;
        uint64_t total = inUse + idle + iowait + irq + softirq;
        *inUseAll += inUse;
        *totalAll += total;
    }

    return NULL;
}
