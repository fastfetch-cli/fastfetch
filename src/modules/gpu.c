#include "fastfetch.h"
#include "common/printing.h"
#include "detection/host/host.h"
#include "detection/gpu/gpu.h"

#include <stdlib.h>

#define FF_GPU_MODULE_NAME "GPU"
#define FF_GPU_NUM_FORMAT_ARGS 5

static void printGPUResult(FFinstance* instance, uint8_t index, FFGPUResult* gpu)
{
    if(instance->config.gpu.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpu.key);

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
        ffPrintFormat(instance, FF_GPU_MODULE_NAME, index, &instance->config.gpu, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->driver},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &gpu->temperature},
            {FF_FORMAT_ARG_TYPE_INT, &gpu->coreCount},
        });
    }
}

void ffPrintGPU(FFinstance* instance)
{
    const FFlist* gpus = ffDetectGPU(instance);

    if(gpus->length == 0)
    {
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpu, "No GPUs found");
        return;
    }

    for(uint8_t i = 0; i < (uint8_t) gpus->length; i++)
        printGPUResult(instance, gpus->length == 1 ? 0 : (uint8_t) (i + 1), ffListGet(gpus, i));
}
