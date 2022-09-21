#include "fastfetch.h"
#include "cpuUsage.h"

#include <unistd.h>
#include <stdio.h>

static const char* getCpuUsageInfo(FILE* procStat, long* inUseAll, long* totalAll)
{
    long user, nice, system, idle, iowait, irq, softirq;

    if (fscanf(procStat, "cpu%ld%ld%ld%ld%ld%ld%ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq) < 0)
    {
        fclose(procStat);
        return "fscanf() failed";
    }
    *inUseAll = user + nice + system;
    *totalAll = *inUseAll + idle + iowait + irq + softirq;
    return NULL;
}

const char* ffGetCpuUsagePercent(double* result)
{
    FILE* procStat = fopen("/proc/stat", "r");
    if(procStat == NULL)
        return "fopen(\"""/proc/stat\", \"r\") == NULL";

    const char* error = NULL;
    long inUseAll1 = 0, totalAll1 = 0;
    error = getCpuUsageInfo(procStat, &inUseAll1, &totalAll1);
    if(error)
        goto exit;

    sleep(1);
    rewind(procStat);

    long inUseAll2 = 0, totalAll2 = 0;
    error = getCpuUsageInfo(procStat, &inUseAll2, &totalAll2);
    if(error)
        goto exit;

    *result = (double)(inUseAll2 - inUseAll1) / (double)(totalAll2 - totalAll1) * 100;

exit:
    fclose(procStat);
    return error;
}
