#include "fastfetch.h"
#include "common/printing.h"
#include "detection/cpuUsage/cpuUsage.h"
#include "sys/time.h"

#define FF_CPU_USAGE_MODULE_NAME "CPU Usage"
#define FF_CPU_USAGE_NUM_FORMAT_ARGS 1

time_t getTimeInMs()
{
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    return (timeNow.tv_sec * 1000) + (timeNow.tv_usec / 1000);
}

static long inUseAll1, totalAll1, startTime;

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
        nanosleep(&(struct timespec){ 1, 0 }, NULL);
    }
    else
    {
        time_t duration = getTimeInMs() - startTime;
        if(duration < 1000)
            nanosleep(&(struct timespec){ 0, (1000 - duration) * 1000000L }, NULL);
    }

    long inUseAll2, totalAll2;
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
