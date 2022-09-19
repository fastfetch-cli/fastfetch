#include "fastfetch.h"
#include "common/printing.h"
#include "common/caching.h"
#include "detection/cpu/cpu.h"

#define FF_CPU_MODULE_NAME "CPU"
#define FF_CPU_NUM_FORMAT_ARGS 8

void ffPrintCPU(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_CPU_MODULE_NAME, &instance->config.cpu, FF_CPU_NUM_FORMAT_ARGS))
        return;

    const FFCPUResult* cpu = ffDetectCPU();

    if(cpu->vendor.length == 0 && cpu->name.length == 0 && cpu->coresOnline <= 1)
    {
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpu, "No CPU detected");
        return;
    }

    FFstrbuf output;
    ffStrbufInitA(&output, 128);

    if(cpu->name.length > 0)
        ffStrbufAppend(&output, &cpu->name);
    else if(cpu->vendor.length > 0)
    {
        ffStrbufAppend(&output, &cpu->vendor);
        ffStrbufAppendS(&output, " CPU");
    }
    else
        ffStrbufAppendS(&output, "CPU");

    if(cpu->coresOnline > 1)
        ffStrbufAppendF(&output, " (%u)", cpu->coresOnline);

    if(cpu->frequencyMax > 0.0)
        ffStrbufAppendF(&output, " @ %.9gGHz", cpu->frequencyMax);

    ffPrintAndWriteToCache(instance, FF_CPU_MODULE_NAME, &instance->config.cpu, &output, FF_CPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &cpu->name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &cpu->vendor},
        {FF_FORMAT_ARG_TYPE_UINT16, &cpu->coresPhysical},
        {FF_FORMAT_ARG_TYPE_UINT16, &cpu->coresLogical},
        {FF_FORMAT_ARG_TYPE_UINT16, &cpu->coresOnline},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu->frequencyMin},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu->frequencyMax},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu->temperature}
    });

    ffStrbufDestroy(&output);
}
