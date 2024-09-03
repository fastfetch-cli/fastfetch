#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/opencl/opencl.h"
#include "detection/gpu/gpu.h"
#include "modules/opencl/opencl.h"
#include "util/stringUtils.h"

#define FF_OPENCL_NUM_FORMAT_ARGS 3

void ffPrintOpenCL(FFOpenCLOptions* options)
{
    FFOpenCLResult* result = ffDetectOpenCL();

    if(result->error != NULL)
    {
        ffPrintError(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", result->error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&result->version, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_OPENCL_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(result->version, "version"),
            FF_FORMAT_ARG(result->name, "name"),
            FF_FORMAT_ARG(result->vendor, "vendor"),
        }));
    }
}

bool ffParseOpenCLCommandOptions(FFOpenCLOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_OPENCL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseOpenCLJsonObject(FFOpenCLOptions* options, yyjson_val* module)
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

        ffPrintError(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateOpenCLJsonConfig(FFOpenCLOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyOpenCLOptions))) FFOpenCLOptions defaultOptions;
    ffInitOpenCLOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateOpenCLJsonResult(FF_MAYBE_UNUSED FFOpenCLOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFOpenCLResult* result = ffDetectOpenCL();

    if(result->error != NULL)
    {
        yyjson_mut_obj_add_str(doc, module, "error", result->error);
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result->version);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &result->name);
    yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &result->vendor);

    yyjson_mut_val* gpus = yyjson_mut_obj_add_arr(doc, obj, "gpus");
    FF_LIST_FOR_EACH(FFGPUResult, gpu, result->gpus)
    {
        yyjson_mut_val* gpuObj = yyjson_mut_arr_add_obj(doc, gpus);
        yyjson_mut_obj_add_str(doc, gpuObj, "type", gpu->type == FF_GPU_TYPE_UNKNOWN ? "Unknown" : gpu->type == FF_GPU_TYPE_INTEGRATED ? "Integrated" : "Discrete");
        yyjson_mut_obj_add_strbuf(doc, gpuObj, "vendor", &gpu->vendor);
        yyjson_mut_obj_add_strbuf(doc, gpuObj, "name", &gpu->name);
        yyjson_mut_obj_add_strbuf(doc, gpuObj, "driver", &gpu->driver);
        yyjson_mut_obj_add_strbuf(doc, gpuObj, "platformApi", &gpu->platformApi);
        if (gpu->coreCount != FF_GPU_CORE_COUNT_UNSET)
            yyjson_mut_obj_add_int(doc, gpuObj, "coreCount", gpu->coreCount);
        else
            yyjson_mut_obj_add_null(doc, gpuObj, "coreCount");

        yyjson_mut_obj_add_uint(doc, gpuObj, "frequency", gpu->frequency);

        yyjson_mut_val* memoryObj = yyjson_mut_obj_add_obj(doc, gpuObj, "memory");

        {
            yyjson_mut_val* dedicatedMemory = yyjson_mut_obj_add_obj(doc, memoryObj, "dedicated");
            if (gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET)
                yyjson_mut_obj_add_uint(doc, dedicatedMemory, "total", gpu->dedicated.total);
            else
                yyjson_mut_obj_add_null(doc, dedicatedMemory, "total");

            if (gpu->dedicated.used != FF_GPU_VMEM_SIZE_UNSET)
                yyjson_mut_obj_add_uint(doc, dedicatedMemory, "used", gpu->dedicated.total);
            else
                yyjson_mut_obj_add_null(doc, dedicatedMemory, "used");
        }

        {
            yyjson_mut_val* sharedMemory = yyjson_mut_obj_add_obj(doc, memoryObj, "shared");
            if (gpu->shared.total != FF_GPU_VMEM_SIZE_UNSET)
                yyjson_mut_obj_add_uint(doc, sharedMemory, "total", gpu->shared.total);
            else
                yyjson_mut_obj_add_null(doc, sharedMemory, "total");

            if (gpu->shared.used != FF_GPU_VMEM_SIZE_UNSET)
                yyjson_mut_obj_add_uint(doc, sharedMemory, "used", gpu->shared.used);
            else
                yyjson_mut_obj_add_null(doc, sharedMemory, "used");
        }

        yyjson_mut_obj_add_uint(doc, gpuObj, "deviceId", gpu->deviceId);
    }
}

void ffPrintOpenCLHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_OPENCL_MODULE_NAME, "{1}", FF_OPENCL_NUM_FORMAT_ARGS, ((const char* []) {
        "Platform version - version",
        "Platform name - name",
        "Platform vendor - vendor",
    }));
}

void ffInitOpenCLOptions(FFOpenCLOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_OPENCL_MODULE_NAME,
        "Print highest OpenCL version supported by the GPU",
        ffParseOpenCLCommandOptions,
        ffParseOpenCLJsonObject,
        ffPrintOpenCL,
        ffGenerateOpenCLJsonResult,
        ffPrintOpenCLHelpFormat,
        ffGenerateOpenCLJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyOpenCLOptions(FFOpenCLOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
