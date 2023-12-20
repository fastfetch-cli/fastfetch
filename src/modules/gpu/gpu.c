#include "common/bar.h"
#include "common/parsing.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/host/host.h"
#include "detection/gpu/gpu.h"
#include "modules/gpu/gpu.h"
#include "util/stringUtils.h"

#include <stdlib.h>

#define FF_GPU_NUM_FORMAT_ARGS 11

static void printGPUResult(FFGPUOptions* options, uint8_t index, const FFGPUResult* gpu)
{
    const char* type;
    switch (gpu->type)
    {
        case FF_GPU_TYPE_INTEGRATED: type = "Integrated"; break;
        case FF_GPU_TYPE_DISCRETE: type = "Discrete"; break;
        default: type = "Unknown"; break;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_GPU_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreateA(gpu->vendor.length + 1 + gpu->name.length);

        if(gpu->vendor.length > 0 && !ffStrbufStartsWith(&gpu->name, &gpu->vendor))
        {
            ffStrbufAppend(&output, &gpu->vendor);
            ffStrbufAppendC(&output, ' ');
        }

        ffStrbufAppend(&output, &gpu->name);

        if(gpu->coreCount != FF_GPU_CORE_COUNT_UNSET)
            ffStrbufAppendF(&output, " (%d)", gpu->coreCount);

        if(gpu->frequency != FF_GPU_FREQUENCY_UNSET)
            ffStrbufAppendF(&output, " @ %.2f MHz", gpu->frequency / 1e6);

        if(gpu->temperature == gpu->temperature) //FF_GPU_TEMP_UNSET
        {
            ffStrbufAppendS(&output, " - ");
            ffParseTemperature(gpu->temperature, &output);
        }

        if(gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET && gpu->dedicated.total != 0)
        {
            ffStrbufAppendS(&output, " (");

            if(gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
            {
                ffParseSize(gpu->dedicated.used, &output);
                ffStrbufAppendS(&output, " / ");
            }
            ffParseSize(gpu->dedicated.total, &output);
            if(gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
            {
                ffStrbufAppendS(&output, ", ");
                ffAppendPercentNum(&output, (double) gpu->dedicated.used / (double) gpu->dedicated.total * 100.0, 50, 80, false);
            }
            ffStrbufAppendC(&output, ')');
        }

        if (gpu->type != FF_GPU_TYPE_UNKNOWN)
            ffStrbufAppendF(&output, " [%s]", type);

        ffStrbufPutTo(&output, stdout);
    }
    else
    {
        ffPrintFormat(FF_GPU_MODULE_NAME, index, &options->moduleArgs, FF_GPU_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gpu->driver},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &gpu->temperature},
            {FF_FORMAT_ARG_TYPE_INT, &gpu->coreCount},
            {FF_FORMAT_ARG_TYPE_STRING, type},
            {FF_FORMAT_ARG_TYPE_UINT64, &gpu->dedicated.total},
            {FF_FORMAT_ARG_TYPE_UINT64, &gpu->dedicated.used},
            {FF_FORMAT_ARG_TYPE_UINT64, &gpu->shared.total},
            {FF_FORMAT_ARG_TYPE_UINT64, &gpu->shared.used},
            {FF_FORMAT_ARG_TYPE_UINT, &gpu->frequency},
        });
    }
}

void ffPrintGPU(FFGPUOptions* options)
{
    FF_LIST_AUTO_DESTROY gpus = ffListCreate(sizeof (FFGPUResult));
    const char* error = ffDetectGPU(options, &gpus);
    if (error)
    {
        ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    FF_LIST_AUTO_DESTROY selectedGPUs;
    ffListInitA(&selectedGPUs, sizeof(const FFGPUResult*), gpus.length);

    FF_LIST_FOR_EACH(FFGPUResult, gpu, gpus)
    {
        if(gpu->type == FF_GPU_TYPE_INTEGRATED && options->hideType == FF_GPU_TYPE_INTEGRATED)
            continue;

        if(gpu->type == FF_GPU_TYPE_DISCRETE && options->hideType == FF_GPU_TYPE_DISCRETE)
            continue;

        * (const FFGPUResult**) ffListAdd(&selectedGPUs) = gpu;
    }

    for(uint32_t i = 0; i < selectedGPUs.length; i++)
        printGPUResult(options, selectedGPUs.length == 1 ? 0 : (uint8_t) (i + 1), * (const FFGPUResult**) ffListGet(&selectedGPUs, i));

    if(selectedGPUs.length == 0)
        ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, "No GPUs found");

    FF_LIST_FOR_EACH(FFGPUResult, gpu, gpus)
    {
        ffStrbufDestroy(&gpu->vendor);
        ffStrbufDestroy(&gpu->name);
        ffStrbufDestroy(&gpu->driver);
    }
}

bool ffParseGPUCommandOptions(FFGPUOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_GPU_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "driver-specific"))
    {
        options->driverSpecific = ffOptionParseBoolean(value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "force-vulkan"))
    {
        options->forceVulkan = ffOptionParseBoolean(value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "temp"))
    {
        options->temp = ffOptionParseBoolean(value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "hide-type"))
    {
        options->hideType = (FFGPUType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "none", FF_GPU_TYPE_UNKNOWN },
            { "integrated", FF_GPU_TYPE_INTEGRATED },
            { "discrete", FF_GPU_TYPE_DISCRETE },
            {},
        });
    }

    return false;
}

void ffParseGPUJsonObject(FFGPUOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffStrEqualsIgnCase(key, "temp"))
        {
            options->temp = yyjson_get_bool(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "driverSpecific"))
        {
            options->driverSpecific = yyjson_get_bool(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "forceVulkan"))
        {
            options->forceVulkan = yyjson_get_bool(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "hideType"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "none", FF_GPU_TYPE_UNKNOWN },
                { "integrated", FF_GPU_TYPE_INTEGRATED },
                { "discrete", FF_GPU_TYPE_DISCRETE },
                {},
            });
            if (error)
                ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, "Invalid %s value: %s", key, error);
            else
                options->hideType = (FFGPUType) value;
            continue;
        }

        ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateGPUJsonConfig(FFGPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyGPUOptions))) FFGPUOptions defaultOptions;
    ffInitGPUOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (options->driverSpecific != defaultOptions.driverSpecific)
        yyjson_mut_obj_add_bool(doc, module, "driverSpecific", options->driverSpecific);

    if (options->forceVulkan != defaultOptions.forceVulkan)
        yyjson_mut_obj_add_bool(doc, module, "forceVulkan", options->forceVulkan);

    if (options->temp != defaultOptions.temp)
        yyjson_mut_obj_add_bool(doc, module, "temp", options->temp);

    if (options->hideType != defaultOptions.hideType)
    {
        switch (options->hideType)
        {
            case FF_GPU_TYPE_UNKNOWN:
                yyjson_mut_obj_add_str(doc, module, "hideType", "none");
                break;
            case FF_GPU_TYPE_INTEGRATED:
                yyjson_mut_obj_add_str(doc, module, "hideType", "integrated");
                break;
            case FF_GPU_TYPE_DISCRETE:
                yyjson_mut_obj_add_str(doc, module, "hideType", "discrete");
                break;
        }
    }
}

void ffGenerateGPUJsonResult(FFGPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY gpus = ffListCreate(sizeof (FFGPUResult));
    const char* error = ffDetectGPU(options, &gpus);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFGPUResult, gpu, gpus)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        if (gpu->coreCount != FF_GPU_CORE_COUNT_UNSET)
            yyjson_mut_obj_add_int(doc, obj, "coreCount", gpu->coreCount);
        else
            yyjson_mut_obj_add_null(doc, obj, "coreCount");

        yyjson_mut_val* memoryObj = yyjson_mut_obj_add_obj(doc, obj, "memory");

        yyjson_mut_val* dedicatedObj = yyjson_mut_obj_add_obj(doc, memoryObj, "dedicated");
        if (gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET)
            yyjson_mut_obj_add_uint(doc, dedicatedObj, "total", gpu->dedicated.total);
        else
            yyjson_mut_obj_add_null(doc, dedicatedObj, "total");

        if (gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
            yyjson_mut_obj_add_uint(doc, dedicatedObj, "used", gpu->dedicated.used);
        else
            yyjson_mut_obj_add_null(doc, dedicatedObj, "used");

        yyjson_mut_obj_add_strbuf(doc, obj, "driver", &gpu->driver);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &gpu->name);

        yyjson_mut_val* sharedObj = yyjson_mut_obj_add_obj(doc, memoryObj, "shared");
        if (gpu->shared.total != FF_GPU_VMEM_SIZE_UNSET)
            yyjson_mut_obj_add_uint(doc, sharedObj, "total", gpu->shared.total);
        else
            yyjson_mut_obj_add_null(doc, sharedObj, "total");
        if (gpu->shared.used != FF_GPU_VMEM_SIZE_UNSET)
            yyjson_mut_obj_add_uint(doc, sharedObj, "used", gpu->shared.used);
        else
            yyjson_mut_obj_add_null(doc, sharedObj, "used");

        if(gpu->temperature == gpu->temperature) //FF_GPU_TEMP_UNSET
            yyjson_mut_obj_add_real(doc, obj, "temperature", gpu->temperature);
        else
            yyjson_mut_obj_add_null(doc, obj, "temperature");

        const char* type;
        switch (gpu->type)
        {
            case FF_GPU_TYPE_INTEGRATED: type = "Integrated"; break;
            case FF_GPU_TYPE_DISCRETE: type = "Discrete"; break;
            default: type = "Unknown"; break;
        }
        yyjson_mut_obj_add_str(doc, obj, "type", type);

        yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &gpu->vendor);

        if (gpu->frequency == FF_GPU_FREQUENCY_UNSET)
            yyjson_mut_obj_add_null(doc, obj, "frequency");
        else
            yyjson_mut_obj_add_uint(doc, obj, "frequency", gpu->frequency);
    }

    FF_LIST_FOR_EACH(FFGPUResult, gpu, gpus)
    {
        ffStrbufDestroy(&gpu->vendor);
        ffStrbufDestroy(&gpu->name);
        ffStrbufDestroy(&gpu->driver);
    }
}

void ffPrintGPUHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_GPU_MODULE_NAME, "{1} {2}", FF_GPU_NUM_FORMAT_ARGS, (const char* []) {
        "GPU vendor",
        "GPU name",
        "GPU driver",
        "GPU temperature",
        "GPU core count",
        "GPU type",
        "GPU total dedicated memory",
        "GPU used dedicated memory",
        "GPU total shared memory",
        "GPU used shared memory",
    });
}

void ffInitGPUOptions(FFGPUOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_GPU_MODULE_NAME,
        "Print GPU names, graphic memory size, type, etc",
        ffParseGPUCommandOptions,
        ffParseGPUJsonObject,
        ffPrintGPU,
        ffGenerateGPUJsonResult,
        ffPrintGPUHelpFormat,
        ffGenerateGPUJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    options->driverSpecific = false;
    options->forceVulkan = false;
    options->temp = false;
    options->hideType = FF_GPU_TYPE_UNKNOWN;
}

void ffDestroyGPUOptions(FFGPUOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
