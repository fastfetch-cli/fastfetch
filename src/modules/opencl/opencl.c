#include "fastfetch.h"
#include "common/printing.h"
#include "detection/opencl/opencl.h"
#include "modules/opencl/opencl.h"

#define FF_OPENCL_NUM_FORMAT_ARGS 3

void ffPrintOpenCL(FFinstance* instance, FFOpenCLOptions* options)
{
    FFOpenCLResult opencl;
    ffStrbufInit(&opencl.version);
    ffStrbufInit(&opencl.device);
    ffStrbufInit(&opencl.vendor);

    const char* error = ffDetectOpenCL(instance, &opencl);

    if(error != NULL)
        ffPrintError(instance, FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs.key);
            ffStrbufPutTo(&opencl.version, stdout);
        }
        else
        {
            ffPrintFormat(instance, FF_OPENCL_MODULE_NAME, 0, &options->moduleArgs, FF_OPENCL_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.version},
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.device},
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.vendor},
            });
        }
    }

    ffStrbufDestroy(&opencl.version);
    ffStrbufDestroy(&opencl.device);
    ffStrbufDestroy(&opencl.vendor);
}

void ffInitOpenCLOptions(FFOpenCLOptions* options)
{
    options->moduleName = FF_OPENCL_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseOpenCLCommandOptions(FFOpenCLOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_OPENCL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyOpenCLOptions(FFOpenCLOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseOpenCLJsonObject(FFinstance* instance, json_object* module)
{
    FFOpenCLOptions __attribute__((__cleanup__(ffDestroyOpenCLOptions))) options;
    ffInitOpenCLOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_OPENCL_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintOpenCL(instance, &options);
}
#endif
