#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/opencl/opencl.h"
#include "modules/opencl/opencl.h"
#include "util/stringUtils.h"

#define FF_OPENCL_NUM_FORMAT_ARGS 3

void ffPrintOpenCL(FFOpenCLOptions* options)
{
    FFOpenCLResult opencl;
    ffStrbufInit(&opencl.version);
    ffStrbufInit(&opencl.device);
    ffStrbufInit(&opencl.vendor);

    const char* error = ffDetectOpenCL(&opencl);

    if(error != NULL)
        ffPrintError(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            ffStrbufPutTo(&opencl.version, stdout);
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_OPENCL_NUM_FORMAT_ARGS, ((FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.version, "version"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.device, "device"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.vendor, "vendor"},
            }));
        }
    }

    ffStrbufDestroy(&opencl.version);
    ffStrbufDestroy(&opencl.device);
    ffStrbufDestroy(&opencl.vendor);
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
    FFOpenCLResult opencl;
    ffStrbufInit(&opencl.version);
    ffStrbufInit(&opencl.device);
    ffStrbufInit(&opencl.vendor);

    const char* error = ffDetectOpenCL(&opencl);

    if(error != NULL)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
    }
    else
    {
        yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
        yyjson_mut_obj_add_strbuf(doc, obj, "version", &opencl.version);
        yyjson_mut_obj_add_strbuf(doc, obj, "device", &opencl.device);
        yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &opencl.vendor);
    }

    ffStrbufDestroy(&opencl.version);
    ffStrbufDestroy(&opencl.device);
    ffStrbufDestroy(&opencl.vendor);
}

void ffPrintOpenCLHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_OPENCL_MODULE_NAME, "{1}", FF_OPENCL_NUM_FORMAT_ARGS, ((const char* []) {
        "version - version",
        "device - device",
        "vendor - vendor"
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
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyOpenCLOptions(FFOpenCLOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
