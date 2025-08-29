#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/temps.h"
#include "common/size.h"
#include "common/frequency.h"
#include "detection/host/host.h"
#include "detection/gpu/gpu.h"
#include "modules/gpu/gpu.h"
#include "util/stringUtils.h"

#include <stdlib.h>

static void printGPUResult(FFGPUOptions* options, uint8_t index, const FFGPUResult* gpu)
{
    const char* type;
    switch (gpu->type)
    {
        case FF_GPU_TYPE_INTEGRATED: type = "Integrated"; break;
        case FF_GPU_TYPE_DISCRETE: type = "Discrete"; break;
        default: type = "Unknown"; break;
    }

    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_GPU_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();

        if(gpu->vendor.length > 0 && !ffStrbufStartsWithIgnCase(&gpu->name, &gpu->vendor))
        {
            ffStrbufAppend(&output, &gpu->vendor);
            ffStrbufAppendC(&output, ' ');
        }

        ffStrbufAppend(&output, &gpu->name);

        if(gpu->coreCount != FF_GPU_CORE_COUNT_UNSET)
            ffStrbufAppendF(&output, " (%d)", gpu->coreCount);

        if(gpu->frequency > 0)
        {
            ffStrbufAppendS(&output, " @ ");
            ffFreqAppendNum(gpu->frequency, &output);
        }

        if(gpu->temperature != FF_GPU_TEMP_UNSET)
        {
            ffStrbufAppendS(&output, " - ");
            ffTempsAppendNum(gpu->temperature, &output, options->tempConfig, &options->moduleArgs);
        }

        if(gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET && gpu->dedicated.total != 0)
        {
            ffStrbufAppendS(&output, " (");

            if (!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
            {
                if(gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
                {
                    ffSizeAppendNum(gpu->dedicated.used, &output);
                    ffStrbufAppendS(&output, " / ");
                }
                ffSizeAppendNum(gpu->dedicated.total, &output);
            }
            if(gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
            {
                double percent = (double) gpu->dedicated.used / (double) gpu->dedicated.total * 100.0;
                if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                {
                    ffStrbufAppendS(&output, ", ");
                    ffPercentAppendNum(&output, percent, options->percent, false, &options->moduleArgs);
                }
                if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                {
                    ffStrbufAppendS(&output, " ");
                    ffPercentAppendBar(&output, percent, options->percent, &options->moduleArgs);
                }
            }
            ffStrbufAppendC(&output, ')');
        }

        if (gpu->type != FF_GPU_TYPE_UNKNOWN)
            ffStrbufAppendF(&output, " [%s]", type);

        ffStrbufPutTo(&output, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY tempStr = ffStrbufCreate();
        ffTempsAppendNum(gpu->temperature, &tempStr, options->tempConfig, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY dTotal = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY dUsed = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY dPercentNum = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY dPercentBar = ffStrbufCreate();
        if (gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET) ffSizeAppendNum(gpu->dedicated.total, &dTotal);
        if (gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET) ffSizeAppendNum(gpu->dedicated.used, &dUsed);
        if (gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET && gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
        {
            double percent = (double) gpu->dedicated.used / (double) gpu->dedicated.total * 100.0;
            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                ffPercentAppendNum(&dPercentNum, percent, options->percent, false, &options->moduleArgs);
            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                ffPercentAppendBar(&dPercentBar, percent, options->percent, &options->moduleArgs);
        }

        FF_STRBUF_AUTO_DESTROY sTotal = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY sUsed = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY sPercentNum = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY sPercentBar = ffStrbufCreate();
        if (gpu->shared.total != FF_GPU_VMEM_SIZE_UNSET) ffSizeAppendNum(gpu->shared.total, &sTotal);
        if (gpu->shared.used != FF_GPU_VMEM_SIZE_UNSET) ffSizeAppendNum(gpu->shared.used, &sUsed);
        if (gpu->shared.total != FF_GPU_VMEM_SIZE_UNSET && gpu->shared.used != FF_GPU_VMEM_SIZE_UNSET)
        {
            double percent = (double) gpu->shared.used / (double) gpu->shared.total * 100.0;
            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                ffPercentAppendNum(&sPercentNum, percent, options->percent, false, &options->moduleArgs);
            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                ffPercentAppendBar(&sPercentBar, percent, options->percent, &options->moduleArgs);
        }

        FF_STRBUF_AUTO_DESTROY frequency = ffStrbufCreate();
        ffFreqAppendNum(gpu->frequency, &frequency);

        FF_STRBUF_AUTO_DESTROY coreUsageNum = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY coreUsageBar = ffStrbufCreate();
        if (gpu->coreUsage != FF_GPU_CORE_USAGE_UNSET)
        {
            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                ffPercentAppendNum(&coreUsageNum, gpu->coreUsage, options->percent, false, &options->moduleArgs);
            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                ffPercentAppendBar(&coreUsageBar, gpu->coreUsage, options->percent, &options->moduleArgs);
        }

        FF_PRINT_FORMAT_CHECKED(FF_GPU_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(gpu->vendor, "vendor"),
            FF_FORMAT_ARG(gpu->name, "name"),
            FF_FORMAT_ARG(gpu->driver, "driver"),
            FF_FORMAT_ARG(tempStr, "temperature"),
            FF_FORMAT_ARG(gpu->coreCount, "core-count"),
            FF_FORMAT_ARG(type, "type"),
            FF_FORMAT_ARG(dTotal, "dedicated-total"),
            FF_FORMAT_ARG(dUsed, "dedicated-used"),
            FF_FORMAT_ARG(sTotal, "shared-total"),
            FF_FORMAT_ARG(sUsed, "shared-used"),
            FF_FORMAT_ARG(gpu->platformApi, "platform-api"),
            FF_FORMAT_ARG(frequency, "frequency"),
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(dPercentNum, "dedicated-percentage-num"),
            FF_FORMAT_ARG(dPercentBar, "dedicated-percentage-bar"),
            FF_FORMAT_ARG(sPercentNum, "shared-percentage-num"),
            FF_FORMAT_ARG(sPercentBar, "shared-percentage-bar"),
            FF_FORMAT_ARG(coreUsageNum, "core-usage-num"),
            FF_FORMAT_ARG(coreUsageBar, "core-usage-bar"),
            FF_FORMAT_ARG(gpu->memoryType, "memory-type"),
        }));
    }
}

bool ffPrintGPU(FFGPUOptions* options)
{
    FF_LIST_AUTO_DESTROY gpus = ffListCreate(sizeof (FFGPUResult));
    const char* error = ffDetectGPU(options, &gpus);
    if (error)
    {
        ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    FF_LIST_AUTO_DESTROY selectedGPUs;
    ffListInitA(&selectedGPUs, sizeof(const FFGPUResult*), gpus.length);

    FF_LIST_FOR_EACH(FFGPUResult, gpu, gpus)
    {
        if(gpu->type == FF_GPU_TYPE_UNKNOWN && options->hideType == FF_GPU_TYPE_UNKNOWN)
            continue;

        if(gpu->type == FF_GPU_TYPE_INTEGRATED && options->hideType == FF_GPU_TYPE_INTEGRATED)
            continue;

        if(gpu->type == FF_GPU_TYPE_DISCRETE && options->hideType == FF_GPU_TYPE_DISCRETE)
            continue;

        * (const FFGPUResult**) ffListAdd(&selectedGPUs) = gpu;
    }

    for(uint32_t i = 0; i < selectedGPUs.length; i++)
        printGPUResult(options, selectedGPUs.length == 1 ? 0 : (uint8_t) (i + 1), *FF_LIST_GET(const FFGPUResult*, selectedGPUs, i));

    if(selectedGPUs.length == 0)
        ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No GPUs found");

    FF_LIST_FOR_EACH(FFGPUResult, gpu, gpus)
    {
        ffStrbufDestroy(&gpu->vendor);
        ffStrbufDestroy(&gpu->name);
        ffStrbufDestroy(&gpu->driver);
        ffStrbufDestroy(&gpu->platformApi);
        ffStrbufDestroy(&gpu->memoryType);
    }

    return true;
}

void ffParseGPUJsonObject(FFGPUOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffTempsParseJsonObject(key, val, &options->temp, &options->tempConfig))
            continue;

        if (unsafe_yyjson_equals_str(key, "driverSpecific"))
        {
            options->driverSpecific = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "detectionMethod"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "auto", FF_GPU_DETECTION_METHOD_AUTO },
                { "pci", FF_GPU_DETECTION_METHOD_PCI },
                { "vulkan", FF_GPU_DETECTION_METHOD_VULKAN },
                { "opencl", FF_GPU_DETECTION_METHOD_OPENCL },
                { "opengl", FF_GPU_DETECTION_METHOD_OPENGL },
                {},
            });
            if (error)
                ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
            else
                options->detectionMethod = (FFGPUDetectionMethod) value;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "hideType"))
        {
            if (yyjson_is_null(val))
                options->hideType = FF_GPU_TYPE_NONE;
            else
            {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                    { "none", FF_GPU_TYPE_NONE },
                    { "unknown", FF_GPU_TYPE_UNKNOWN },
                    { "integrated", FF_GPU_TYPE_INTEGRATED },
                    { "discrete", FF_GPU_TYPE_DISCRETE },
                    {},
                });
                if (error)
                    ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
                else
                    options->hideType = (FFGPUType) value;
            }
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_GPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateGPUJsonConfig(FFGPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "driverSpecific", options->driverSpecific);

    switch (options->detectionMethod)
    {
        case FF_GPU_DETECTION_METHOD_AUTO:
            yyjson_mut_obj_add_str(doc, module, "detectionMethod", "auto");
            break;
        case FF_GPU_DETECTION_METHOD_PCI:
            yyjson_mut_obj_add_str(doc, module, "detectionMethod", "pci");
            break;
        case FF_GPU_DETECTION_METHOD_VULKAN:
            yyjson_mut_obj_add_str(doc, module, "detectionMethod", "vulkan");
            break;
        case FF_GPU_DETECTION_METHOD_OPENCL:
            yyjson_mut_obj_add_str(doc, module, "detectionMethod", "opencl");
            break;
        case FF_GPU_DETECTION_METHOD_OPENGL:
            yyjson_mut_obj_add_str(doc, module, "detectionMethod", "opengl");
            break;
    }

    ffTempsGenerateJsonConfig(doc, module, options->temp, options->tempConfig);

    switch (options->hideType)
    {
        case FF_GPU_TYPE_NONE:
            yyjson_mut_obj_add_str(doc, module, "hideType", "none");
            break;
        case FF_GPU_TYPE_UNKNOWN:
            yyjson_mut_obj_add_str(doc, module, "hideType", "unknown");
            break;
        case FF_GPU_TYPE_INTEGRATED:
            yyjson_mut_obj_add_str(doc, module, "hideType", "integrated");
            break;
        case FF_GPU_TYPE_DISCRETE:
            yyjson_mut_obj_add_str(doc, module, "hideType", "discrete");
            break;
    }

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateGPUJsonResult(FFGPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY gpus = ffListCreate(sizeof (FFGPUResult));
    const char* error = ffDetectGPU(options, &gpus);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFGPUResult, gpu, gpus)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);

        if (gpu->index != FF_GPU_INDEX_UNSET)
            yyjson_mut_obj_add_uint(doc, obj, "index", gpu->index);
        else
            yyjson_mut_obj_add_null(doc, obj, "index");

        if (gpu->coreCount != FF_GPU_CORE_COUNT_UNSET)
            yyjson_mut_obj_add_int(doc, obj, "coreCount", gpu->coreCount);
        else
            yyjson_mut_obj_add_null(doc, obj, "coreCount");

        if (gpu->coreUsage != FF_GPU_CORE_USAGE_UNSET)
            yyjson_mut_obj_add_real(doc, obj, "coreUsage", gpu->coreUsage);
        else
            yyjson_mut_obj_add_null(doc, obj, "coreUsage");

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

        yyjson_mut_val* sharedObj = yyjson_mut_obj_add_obj(doc, memoryObj, "shared");
        if (gpu->shared.total != FF_GPU_VMEM_SIZE_UNSET)
            yyjson_mut_obj_add_uint(doc, sharedObj, "total", gpu->shared.total);
        else
            yyjson_mut_obj_add_null(doc, sharedObj, "total");
        if (gpu->shared.used != FF_GPU_VMEM_SIZE_UNSET)
            yyjson_mut_obj_add_uint(doc, sharedObj, "used", gpu->shared.used);
        else
            yyjson_mut_obj_add_null(doc, sharedObj, "used");

        if (gpu->memoryType.length)
            yyjson_mut_obj_add_strbuf(doc, memoryObj, "type", &gpu->memoryType);
        else
            yyjson_mut_obj_add_null(doc, memoryObj, "type");

        yyjson_mut_obj_add_strbuf(doc, obj, "driver", &gpu->driver);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &gpu->name);

        if(gpu->temperature != FF_GPU_TEMP_UNSET)
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

        yyjson_mut_obj_add_strbuf(doc, obj, "platformApi", &gpu->platformApi);

        if (gpu->frequency != FF_GPU_FREQUENCY_UNSET)
            yyjson_mut_obj_add_uint(doc, obj, "frequency", gpu->frequency);
        else
            yyjson_mut_obj_add_null(doc, obj, "frequency");

        yyjson_mut_obj_add_uint(doc, obj, "deviceId", gpu->deviceId);
    }

    FF_LIST_FOR_EACH(FFGPUResult, gpu, gpus)
    {
        ffStrbufDestroy(&gpu->vendor);
        ffStrbufDestroy(&gpu->name);
        ffStrbufDestroy(&gpu->driver);
        ffStrbufDestroy(&gpu->platformApi);
        ffStrbufDestroy(&gpu->memoryType);
    }

    return true;
}

void ffInitGPUOptions(FFGPUOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰾲");

    options->driverSpecific = false;
    options->detectionMethod =
        #if defined(__x86_64__) || defined(__i386__)
        FF_GPU_DETECTION_METHOD_PCI
        #else
        FF_GPU_DETECTION_METHOD_AUTO
        #endif
    ;
    options->temp = false;
    options->hideType = FF_GPU_TYPE_NONE;
    options->tempConfig = (FFColorRangeConfig) { 60, 80 };
    options->percent = (FFPercentageModuleConfig) { 50, 80, 0 };
}

void ffDestroyGPUOptions(FFGPUOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffGPUModuleInfo = {
    .name = FF_GPU_MODULE_NAME,
    .description = "Print GPU names, graphic memory size, type, etc",
    .initOptions = (void*) ffInitGPUOptions,
    .destroyOptions = (void*) ffDestroyGPUOptions,
    .parseJsonObject = (void*) ffParseGPUJsonObject,
    .printModule = (void*) ffPrintGPU,
    .generateJsonResult = (void*) ffGenerateGPUJsonResult,
    .generateJsonConfig = (void*) ffGenerateGPUJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"GPU vendor", "vendor"},
        {"GPU name", "name"},
        {"GPU driver", "driver"},
        {"GPU temperature", "temperature"},
        {"GPU core count", "core-count"},
        {"GPU type", "type"},
        {"GPU total dedicated memory", "dedicated-total"},
        {"GPU used dedicated memory", "dedicated-used"},
        {"GPU total shared memory", "shared-total"},
        {"GPU used shared memory", "shared-used"},
        {"The platform API used when detecting the GPU", "platform-api"},
        {"Current frequency in GHz", "frequency"},
        {"GPU vendor specific index", "index"},
        {"Dedicated memory usage percentage num", "dedicated-percentage-num"},
        {"Dedicated memory usage percentage bar", "dedicated-percentage-bar"},
        {"Shared memory usage percentage num", "shared-percentage-num"},
        {"Shared memory usage percentage bar", "shared-percentage-bar"},
        {"Core usage percentage num", "core-usage-num"},
        {"Core usage percentage bar", "core-usage-bar"},
        {"Memory type (Windows only)", "memory-type"},
    })),
};
