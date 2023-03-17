#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"
#include "common/time.h"

#include <stdint.h>

// We need to use uint64_t because sizeof(long) == 4 on Windows
const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll);

static uint64_t inUseAll1, totalAll1;

void ffPrepareCPUUsage()
{
    ffGetCpuUsageInfo(&inUseAll1, &totalAll1);
}

const char* ffGetCpuUsageResult(double* result)
{
    const char* error = NULL;
    if(inUseAll1 == 0 && totalAll1 == 0)
    {
        error = ffGetCpuUsageInfo(&inUseAll1, &totalAll1);
        if(error)
            return error;
        ffTimeSleep(200);
    }

    while(true)
    {
        uint64_t inUseAll2, totalAll2;
        error = ffGetCpuUsageInfo(&inUseAll2, &totalAll2);
        if(error)
            return error;

        if(inUseAll2 != inUseAll1)
        {
            *result = (double)(inUseAll2 - inUseAll1) / (double)(totalAll2 - totalAll1) * 100;
            return NULL;
        }
        else
            ffTimeSleep(200);
    }
}
