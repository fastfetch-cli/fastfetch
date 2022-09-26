#include "fastfetch.h"
#include "common/printing.h"
#include "common/caching.h"
#include "detection/cpu/cpu.h"

#define FF_CPU_MODULE_NAME "CPU"
#define FF_CPU_NUM_FORMAT_ARGS 8

void ffPrintCPU(FFinstance* instance)
{
    const FFCPUResult* cpu = ffDetectCPU(instance);

    if(cpu->vendor.length == 0 && cpu->name.length == 0 && cpu->coresOnline <= 1)
    {
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpu, "No CPU detected");
        return;
    }

    if(instance->config.cpu.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpu.key);

        if(cpu->name.length > 0)
            ffStrbufWriteTo(&cpu->name, stdout);
        else if(cpu->vendor.length > 0)
        {
            ffStrbufWriteTo(&cpu->vendor, stdout);
            fputs(" CPU", stdout);
        }
        else
            fputs("CPU", stdout);

        if(cpu->coresOnline > 1)
            printf(" (%u)", cpu->coresOnline);

        if(cpu->frequencyMax > 0.0)
            printf(" @ %.9g GHz", cpu->frequencyMax);

        if(cpu->temperature == cpu->temperature) //FF_CPU_TEMP_UNSET
            printf(" - %.1f°C", cpu->temperature);

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpu, FF_CPU_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &cpu->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &cpu->vendor},
            {FF_FORMAT_ARG_TYPE_UINT16, &cpu->coresPhysical},
            {FF_FORMAT_ARG_TYPE_UINT16, &cpu->coresLogical},
            {FF_FORMAT_ARG_TYPE_UINT16, &cpu->coresOnline},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu->frequencyMin},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu->frequencyMax},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu->temperature}
        });
    }
}
