#include "fastfetch.h"
#include "cpuUsage.h"

#ifndef FF_DETECTION_CPUUSAGE_NOWAIT

#include "common/time.h"

#include <stdint.h>

// We need to use uint64_t because sizeof(long) == 4 on Windows
const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll);

static uint64_t inUseAll1, totalAll1, startTime;

void ffPrepareCPUUsage()
{
    startTime = ffTimeGetTick();
    ffGetCpuUsageInfo(&inUseAll1, &totalAll1);
}

const char* ffGetCpuUsageResult(double* result)
{
    const char* error = NULL;
    if(startTime == 0)
    {
        error = ffGetCpuUsageInfo(&inUseAll1, &totalAll1);
        if(error)
            return error;
        ffTimeSleep(1000);
    }
    else
    {
        uint64_t duration = ffTimeGetTick() - startTime;
        if(duration < 1000)
            ffTimeSleep(1000 - (uint32_t) duration);
    }

    uint64_t inUseAll2, totalAll2;
    error = ffGetCpuUsageInfo(&inUseAll2, &totalAll2);
    if(error)
        return error;

    *result = (double)(inUseAll2 - inUseAll1) / (double)(totalAll2 - totalAll1) * 100;

    return NULL;
}

#else //FF_DETECTION_CPUUSAGE_NOWAIT

const char* ffGetCpuUsageResultNoWait(double* result);

void ffPrepareCPUUsage() {}

const char* ffGetCpuUsageResult(double* result) {
    return ffGetCpuUsageResultNoWait(result);
}

#endif //FF_DETECTION_CPUUSAGE_NOWAIT
