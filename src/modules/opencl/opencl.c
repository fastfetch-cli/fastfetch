#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/opencl/opencl.h"
#include "detection/gpu/gpu.h"
#include "modules/opencl/opencl.h"
#include "util/stringUtils.h"

bool ffPrintOpenCL(FFOpenCLOptions* options)
{
    FFOpenCLResult* result = ffDetectOpenCL();

    if(result->error != NULL)
    {
        ffPrintError(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", result->error);
        return false;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&result->version, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result->version, "version"),
            FF_FORMAT_ARG(result->name, "name"),
            FF_FORMAT_ARG(result->vendor, "vendor"),
        }));
    }

    return true;
}

void ffParseOpenCLJsonObject(FFOpenCLOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateOpenCLJsonConfig(FFOpenCLOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateOpenCLJsonResult(FF_MAYBE_UNUSED FFOpenCLOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFOpenCLResult* result = ffDetectOpenCL();

    if(result->error != NULL)
    {
        yyjson_mut_obj_add_str(doc, module, "error", result->error);
        return false;
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

    return true;
}

void ffInitOpenCLOptions(FFOpenCLOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyOpenCLOptions(FFOpenCLOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffOpenCLModuleInfo = {
    .name = FF_OPENCL_MODULE_NAME,
    .description = "Print highest OpenCL version supported by the GPU",
    .initOptions = (void*) ffInitOpenCLOptions,
    .destroyOptions = (void*) ffDestroyOpenCLOptions,
    .parseJsonObject = (void*) ffParseOpenCLJsonObject,
    .printModule = (void*) ffPrintOpenCL,
    .generateJsonResult = (void*) ffGenerateOpenCLJsonResult,
    .generateJsonConfig = (void*) ffGenerateOpenCLJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Platform version", "version"},
        {"Platform name", "name"},
        {"Platform vendor", "vendor"},
    }))
};
