#include "fastfetch.h"
#include "common/bar.h"
#include "common/parsing.h"
#include "common/printing.h"
#include "detection/host/host.h"
#include "detection/gpu/gpu.h"
#include "modules/gpu/gpu.h"

#include <stdlib.h>

#define FF_GPU_NUM_FORMAT_ARGS 6

static void printGPUResult(FFinstance* instance, FFGPUOptions* options, uint8_t index, const FFGPUResult* gpu)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_GPU_MODULE_NAME, index, &options->moduleArgs.key);

        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreateA(gpu->vendor.length + 1 + gpu->name.length);

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

        if(gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET && gpu->dedicated.total != 0)
        {
            ffStrbufAppendS(&output, " (");

            if(gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
            {
                ffParseSize(gpu->dedicated.used, instance->config.binaryPrefixType, &output);
                ffStrbufAppendS(&output, " / ");
            }
            ffParseSize(gpu->dedicated.total, instance->config.binaryPrefixType, &output);
            if(gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
            {
                ffStrbufAppendS(&output, ", ");
                ffAppendPercentNum(instance, &output, (uint8_t) (gpu->dedicated.used * 100 / gpu->dedicated.total), 50, 80, false);
            }
            ffStrbufAppendC(&output, ')');
        }

        ffStrbufPutTo(&output, stdout);
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

        ffPrintFormat(instance, FF_GPU_MODULE_NAME, index, &options->moduleArgs, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->driver},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &gpu->temperature},
            {FF_FORMAT_ARG_TYPE_INT, &gpu->coreCount},
            {FF_FORMAT_ARG_TYPE_STRING, type},
        });
    }
}

void ffPrintGPU(FFinstance* instance, FFGPUOptions* options)
{
    const FFlist* gpus = ffDetectGPU(instance);

    FF_LIST_AUTO_DESTROY selectedGPUs;
    ffListInitA(&selectedGPUs, sizeof(const FFGPUResult*), gpus->length);

    FF_LIST_FOR_EACH(FFGPUResult, gpu, *gpus)
    {
        if(gpu->type == FF_GPU_TYPE_INTEGRATED && options->hideType == FF_GPU_TYPE_INTEGRATED)
            continue;

        if(gpu->type == FF_GPU_TYPE_DISCRETE && options->hideType == FF_GPU_TYPE_DISCRETE)
            continue;

        * (const FFGPUResult**) ffListAdd(&selectedGPUs) = gpu;
    }

    for(uint32_t i = 0; i < selectedGPUs.length; i++)
        printGPUResult(instance, options, selectedGPUs.length == 1 ? 0 : (uint8_t) (i + 1), * (const FFGPUResult**) ffListGet(&selectedGPUs, i));

    if(selectedGPUs.length == 0)
        ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &options->moduleArgs, "No GPUs found");
}

void ffInitGPUOptions(FFGPUOptions* options)
{
    options->moduleName = FF_GPU_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    options->forceVulkan = false;
    options->temp = false;
    options->hideType = FF_GPU_TYPE_UNKNOWN;
}

bool ffParseGPUCommandOptions(FFGPUOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_GPU_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "force-vulkan") == 0)
    {
        options->forceVulkan = ffOptionParseBoolean(value);
        return true;
    }

    if (strcasecmp(subKey, "temp") == 0)
    {
        options->temp = ffOptionParseBoolean(value);
        return true;
    }

    if (strcasecmp(subKey, "hide-type") == 0)
    {
        options->hideType = (FFGPUType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "none", FF_GPU_TYPE_UNKNOWN },
            { "intergrated", FF_GPU_TYPE_INTEGRATED },
            { "discrete", FF_GPU_TYPE_DISCRETE },
            {},
        });
    }

    return false;
}

void ffDestroyGPUOptions(FFGPUOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseGPUJsonObject(FFinstance* instance, json_object* module)
{
    FFGPUOptions __attribute__((__cleanup__(ffDestroyGPUOptions))) options;
    ffInitGPUOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "temp") == 0)
            {
                options.temp = (bool) json_object_get_boolean(val);
                continue;
            }

            if (strcasecmp(key, "forceVulkan") == 0)
            {
                options.forceVulkan = (bool) json_object_get_boolean(val);
                continue;
            }

            if (strcasecmp(key, "hideType") == 0)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                    { "none", FF_GPU_TYPE_UNKNOWN },
                    { "intergrated", FF_GPU_TYPE_INTEGRATED },
                    { "discrete", FF_GPU_TYPE_DISCRETE },
                    {},
                });
                if (error)
                    ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &options.moduleArgs, "Invalid %s value: %s", key, error);
                else
                    options.hideType = (FFGPUType) value;
                continue;
            }

            ffPrintError(instance, FF_GPU_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintGPU(instance, &options);
}
#endif
