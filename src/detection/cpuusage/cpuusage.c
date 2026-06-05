#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"
#include "common/time.h"

#include <stdint.h>

static FFlist cpuTimes1;
static uint64_t startTime;

void ffPrepareCPUUsage(void) {
    if (startTime != 0) {
        return; // Already prepared
    }

    ffListInit(&cpuTimes1);
    ffGetCpuUsageInfo(&cpuTimes1);
    startTime = ffTimeGetNow();
}

const char* ffGetCpuUsageResult(FFCPUUsageOptions* options, FFlist* result) {
    const char* error = NULL;
    if (startTime == 0) {
        ffListInit(&cpuTimes1);
        error = ffGetCpuUsageInfo(&cpuTimes1);
        if (error) {
            return error;
        }
        ffTimeSleep(options->waitTime);
    } else {
        uint64_t elapsedTime = ffTimeGetNow() - startTime;
        if (elapsedTime < options->waitTime) {
            ffTimeSleep(options->waitTime - (uint32_t) elapsedTime);
        }
    }

    if (cpuTimes1.length == 0) {
        return "No CPU cores found";
    }

    FF_LIST_AUTO_DESTROY cpuTimes2 = ffListCreate();
    uint32_t retryCount = 0;

retry:
    error = ffGetCpuUsageInfo(&cpuTimes2);
    if (error) {
        return error;
    }
    if (cpuTimes1.length != cpuTimes2.length) {
        return "Unexpected CPU usage result";
    }

    for (uint32_t i = 0; i < cpuTimes1.length; ++i) {
        FFCpuUsageInfo* cpuTime1 = FF_LIST_GET(FFCpuUsageInfo, cpuTimes1, i);
        FFCpuUsageInfo* cpuTime2 = FF_LIST_GET(FFCpuUsageInfo, cpuTimes2, i);
        if (cpuTime2->totalAll <= cpuTime1->totalAll) {
            if (++retryCount <= 3) {
                ffListClear(&cpuTimes2);
                ffTimeSleep(options->waitTime);
                goto retry;
            }
            return "CPU time did not increase. Try increasing wait time.";
        }
    }

    for (uint32_t i = 0; i < cpuTimes1.length; ++i) {
        FFCpuUsageInfo* cpuTime1 = FF_LIST_GET(FFCpuUsageInfo, cpuTimes1, i);
        FFCpuUsageInfo* cpuTime2 = FF_LIST_GET(FFCpuUsageInfo, cpuTimes2, i);
        *FF_LIST_ADD(double, *result) = (double) (cpuTime2->inUseAll - cpuTime1->inUseAll) / (double) (cpuTime2->totalAll - cpuTime1->totalAll) * 100;
        cpuTime1->inUseAll = cpuTime2->inUseAll;
        cpuTime1->totalAll = cpuTime2->totalAll;
    }
    startTime = ffTimeGetNow();
    return NULL;
}
