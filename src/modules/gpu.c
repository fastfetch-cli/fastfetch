#include "fastfetch.h"
#include "common/printing.h"
#include "detection/host/host.h"
#include "detection/gpu/gpu.h"

#include <stdlib.h>

#define FF_GPU_MODULE_NAME "GPU"
#define FF_GPU_NUM_FORMAT_ARGS 6

static void printGPUResult(FFinstance* instance, uint8_t index, const FFGPUResult* gpu)
{
    if(instance->config.gpu.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_GPU_MODULE_NAME, index, &instance->config.gpu.key);

        FFstrbuf output;
        ffStrbufInitA(&output, gpu->vendor.length + 1 + gpu->name.length);

        if(gpu->vendor.length > 0 && !ffStrbufStartsWith(&gpu->name, &gpu->vendor))
        {
            ffStrbufAppend(&output, &gpu->vendor);
            ffStrbufAppendC(&output, ' ');
        }

        ffStrbufAppend(&output, &gpu->name);

        if(gpu->coreCount != FF_GPU_CORE_COUNT_UNSET)
            ffStrbufAppendF(&output, " (%d)", gpu->coreCount);

        if(gpu->temperature == gpu->temperature) //FF_GPU_TEMP_UNSET
            ffStrbufAppendF(&output, " - %.1f°C", gpu->temperature);

        ffStrbufPutTo(&output, stdout);

        ffStrbufDestroy(&output);
    }
    else
    {
        const char* type;
        if(gpu->type == FF_GPU_TYPE_INTEGRATED)
            type = "Integrated";
        else if(gpu->type == FF_GPU_TYPE_DISCRETE)
            type = "Discrete";
        else
            type = "Unknown";

        ffPrintFormat(instance, FF_GPU_MODULE_NAME, index, &instance->config.gpu, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->driver},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &gpu->temperature},
            {FF_FORMAT_ARG_TYPE_INT, &gpu->coreCount},
            {FF_FORMAT_ARG_TYPE_STRING, type},
        });
    }
}

void ffPrintGPU(FFinstance* instance)
{
    const FFlist* gpus = ffDetectGPU(instance);

    FFlist selectedGpus;
    ffListInitA(&selectedGpus, sizeof(const FFGPUResult*), gpus->length);

    for(uint32_t i = 0; i < gpus->length; i++)
    {
        const FFGPUResult* gpu = ffListGet(gpus, i);

        if(gpu->type == FF_GPU_TYPE_INTEGRATED && instance->config.gpuHideIntegrated)
            continue;

        if(gpu->type == FF_GPU_TYPE_DISCRETE && instance->config.gpuHideDiscrete)
            continue;

        * (const FFGPUResult**) ffListAdd(&selectedGpus) = gpu;
    }

    for(uint32_t i = 0; i < selectedGpus.length; i++)
        printGPUResult(instance, selectedGpus.length == 1 ? 0 : (uint8_t) (i + 1), * (const FFGPUResult**) ffListGet(&selectedGpus, i));

    if(selectedGpus.length == 0)
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpu, "No GPUs found");

    ffListDestroy(&selectedGpus);
}
