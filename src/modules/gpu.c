#include "fastfetch.h"
#include "common/printing.h"
#include "common/caching.h"
#include "detection/host/host.h"
#include "detection/gpu/gpu.h"

#include <stdlib.h>

#define FF_GPU_MODULE_NAME "GPU"
#define FF_GPU_NUM_FORMAT_ARGS 5

static void printGPUResult(FFinstance* instance, uint8_t index, FFcache* cache, FFGPUResult* gpu)
{
    FFstrbuf output;
    ffStrbufInitA(&output, gpu->vendor.length + 1 + gpu->name.length);

    if(gpu->vendor.length > 0)
    {
        ffStrbufAppend(&output, &gpu->vendor);
        ffStrbufAppendC(&output, ' ');
    }

    ffStrbufAppend(&output, &gpu->name);

    if(gpu->coreCount != FF_GPU_CORE_COUNT_UNSET)
        ffStrbufAppendF(&output, " (%d)", gpu->coreCount);

    ffPrintAndAppendToCache(instance, FF_GPU_MODULE_NAME, index, &instance->config.gpu, cache, &output, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->vendor},
        {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->driver},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &gpu->temperature},
        {FF_FORMAT_ARG_TYPE_INT, &gpu->coreCount},
    });

    ffStrbufDestroy(&output);
}

void ffPrintGPU(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_GPU_MODULE_NAME, &instance->config.gpu, FF_GPU_NUM_FORMAT_ARGS))
        return;

    if(ffStrbufCompS(&ffDetectHost()->productName, FF_HOST_PRODUCT_NAME_WSL) == 0)
    {
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpu, "WSL doesn't expose senseful GPU names");
        return;
    }

    const FFlist* gpus = ffDetectGPU(instance);

    if(gpus->length == 0)
    {
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &instance->config.gpu, "No GPUs found");
        return;
    }

    FFcache cache;
    ffCacheOpenWrite(instance, FF_GPU_MODULE_NAME, &cache);

    for(uint8_t i = 0; i < (uint8_t) gpus->length; i++)
        printGPUResult(instance, gpus->length == 1 ? 0 : (uint8_t) (i + 1), &cache, ffListGet(gpus, i));

    ffCacheClose(&cache);
}
