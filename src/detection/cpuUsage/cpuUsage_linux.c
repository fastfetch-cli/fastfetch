#include "fastfetch.h"
#include "cpuUsage.h"

#include <stdio.h>
#include <inttypes.h>

const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll)
{
    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0;

    FILE* procStat = fopen("/proc/stat", "r");
    if(procStat == NULL)
    {
        #ifdef __ANDROID__
        return "Accessing \"/proc/stat\" is restricted on Android O+";
        #else
        return "fopen(\"""/proc/stat\", \"r\") == NULL";
        #endif
    }

    if (fscanf(procStat, "cpu%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64, &user, &nice, &system, &idle, &iowait, &irq, &softirq) < 0)
    {
        fclose(procStat);
        return "fscanf() failed";
    }
    *inUseAll = user + nice + system;
    *totalAll = *inUseAll + idle + iowait + irq + softirq;

    fclose(procStat);

    return NULL;
}
