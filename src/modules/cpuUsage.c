#include "fastfetch.h"
#include "common/printing.h"
#include "detection/cpuUsage/cpuUsage.h"

#define FF_CPU_USAGE_MODULE_NAME "CPU Usage"
#define FF_CPU_USAGE_NUM_FORMAT_ARGS 1

void ffPrintCPUUsage(FFinstance* instance)
{
    double cpuPercent = 0;
    const char* error = ffGetCpuUsagePercent(&cpuPercent);

    if(error)
    {
        ffPrintError(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpu, "%s", error);
        return;
    }

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
