#include "fastfetch.h"
#include "cpuUsage.h"

#include <stdio.h>

const char* ffGetCpuUsageInfo(long* inUseAll, long* totalAll)
{
    long user, nice, system, idle, iowait, irq, softirq;

    FILE* procStat = fopen("/proc/stat", "r");
    if(procStat == NULL)
        return "fopen(\"""/proc/stat\", \"r\") == NULL";

    if (fscanf(procStat, "cpu%ld%ld%ld%ld%ld%ld%ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq) < 0)
    {
        fclose(procStat);
        return "fscanf() failed";
    }
    *inUseAll = user + nice + system;
    *totalAll = *inUseAll + idle + iowait + irq + softirq;

    fclose(procStat);

    return NULL;
}
