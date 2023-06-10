#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/kernel/kernel.h"

#define FF_KERNEL_NUM_FORMAT_ARGS 4

void ffPrintKernel(FFinstance* instance, FFKernelOptions* options)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs.key);
        ffStrbufWriteTo(&instance->state.platform.systemRelease, stdout);

        #ifdef _WIN32
            if(instance->state.platform.systemVersion.length > 0)
                printf(" (%s)", instance->state.platform.systemVersion.chars);
        #endif

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_KERNEL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemRelease},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemArchitecture}
        });
    }
}

void ffInitKernelOptions(FFKernelOptions* options)
{
    options->moduleName = FF_KERNEL_MODULE_NAME;
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

void ffParseKernelJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFKernelOptions __attribute__((__cleanup__(ffDestroyKernelOptions))) options;
    ffInitKernelOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_KERNEL_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintKernel(instance, &options);
}
