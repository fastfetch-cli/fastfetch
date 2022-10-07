#include "fastfetch.h"
#include "common/printing.h"
#include "detection/cpuUsage/cpuUsage.h"

#if defined(_WIN32) || defined(__CYGWIN__)
    #include <synchapi.h>
    #include <sysinfoapi.h>
#else
    #include <sys/time.h>
    #include <time.h>
#endif

#define FF_CPU_USAGE_MODULE_NAME "CPU Usage"
#define FF_CPU_USAGE_NUM_FORMAT_ARGS 1

static inline uint64_t getTimeInMs()
{
    #if defined(_WIN32) || defined(__CYGWIN__)
        return GetTickCount64();
    #else
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        return (uint64_t)((timeNow.tv_sec * 1000) + (timeNow.tv_usec / 1000));
    #endif
}

static inline void sleepInMs(uint32_t msec)
{
    #if defined(_WIN32) || defined(__CYGWIN__)
        SleepEx(msec, TRUE);
    #else
        nanosleep(&(struct timespec){ msec / 1000, (msec % 1000) * 1000000 }, NULL);
    #endif
}

static uint64_t inUseAll1, totalAll1, startTime;

void ffPrepareCPUUsage()
{
    startTime = getTimeInMs();
    ffGetCpuUsageInfo(&inUseAll1, &totalAll1);
}

void ffPrintCPUUsage(FFinstance* instance)
{
    const char* error = NULL;
    if(startTime == 0)
    {
        error = ffGetCpuUsageInfo(&inUseAll1, &totalAll1);
        if(error)
            goto error;
        sleepInMs(1000);
    }
    else
    {
        uint64_t duration = getTimeInMs() - startTime;
        if(duration < 1000)
            sleepInMs(1000 - (uint32_t) duration);
    }

    uint64_t inUseAll2, totalAll2;
    error = ffGetCpuUsageInfo(&inUseAll2, &totalAll2);

    if(error)
    {
    error:
        ffPrintError(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpu, "%s", error);
        return;
    }

    double cpuPercent = (double)(inUseAll2 - inUseAll1) / (double)(totalAll2 - totalAll1) * 100;

    if(instance->config.cpuUsage.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpuUsage.key);

        printf("%.2lf%%\n", cpuPercent);
    }
    else
    {
        ffPrintFormat(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpuUsage, FF_CPU_USAGE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_DOUBLE, &cpuPercent}
        });
    }
}
