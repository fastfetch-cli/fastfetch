#include "fastfetch.h"
#include "common/printing.h"
#include "common/bar.h"
#include "detection/cpuUsage/cpuUsage.h"

#define FF_CPU_USAGE_MODULE_NAME "CPU Usage"
#define FF_CPU_USAGE_NUM_FORMAT_ARGS 1

void ffPrintCPUUsage(FFinstance* instance)
{
    double percentage = 0.0/0.0;
    const char* error = ffGetCpuUsageResult(&percentage);

    if(error)
    {
        ffPrintError(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpu, "%s", error);
        return;
    }

    if(instance->config.cpuUsage.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpuUsage.key);

        FF_STRBUF_AUTO_DESTROY str;
        ffStrbufInit(&str);
        if(instance->config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffAppendPercentBar(instance, &str, (uint8_t)percentage, 0, 5, 8);
        if(instance->config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
        {
            if(str.length > 0)
                ffStrbufAppendC(&str, ' ');
            ffAppendPercentNum(instance, &str, (uint8_t) percentage, 50, 80, str.length > 0);
        }
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_CPU_USAGE_MODULE_NAME, 0, &instance->config.cpuUsage, FF_CPU_USAGE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_DOUBLE, &percentage}
        });
    }
}
