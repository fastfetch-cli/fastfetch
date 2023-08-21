#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/kernel/kernel.h"
#include "util/stringUtils.h"

#define FF_KERNEL_NUM_FORMAT_ARGS 4

void ffPrintKernel(FFKernelOptions* options)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&instance.state.platform.systemRelease, stdout);

        #ifdef _WIN32
            if(instance.state.platform.systemVersion.length > 0)
                printf(" (%s)", instance.state.platform.systemVersion.chars);
        #endif

        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_KERNEL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance.state.platform.systemName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance.state.platform.systemRelease},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance.state.platform.systemVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance.state.platform.systemArchitecture}
        });
    }
}

void ffInitKernelOptions(FFKernelOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_KERNEL_MODULE_NAME, ffParseKernelCommandOptions, ffParseKernelJsonObject, ffPrintKernel);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseKernelCommandOptions(FFKernelOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_KERNEL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyKernelOptions(FFKernelOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseKernelJsonObject(FFKernelOptions* options, yyjson_val* module)
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

        ffPrintError(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
