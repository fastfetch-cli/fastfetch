#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/kernel/kernel.h"
#include "util/stringUtils.h"

#define FF_KERNEL_NUM_FORMAT_ARGS 4

void ffPrintKernel(FFKernelOptions* options)
{
    const FFPlatform* platform = &instance.state.platform;
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&platform->systemRelease, stdout);

        if(platform->systemDisplayVersion.length > 0)
            printf(" (%s)\n", platform->systemDisplayVersion.chars);
        else
            putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_KERNEL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemRelease},
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemArchitecture},
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemDisplayVersion}
        });
    }
}

void ffInitKernelOptions(FFKernelOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_KERNEL_MODULE_NAME, ffParseKernelCommandOptions, ffParseKernelJsonObject, ffPrintKernel, ffGenerateKernelJson);
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

void ffGenerateKernelJson(FF_MAYBE_UNUSED FFKernelOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "architecture", &instance.state.platform.systemArchitecture);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &instance.state.platform.systemName);
    yyjson_mut_obj_add_strbuf(doc, obj, "release", &instance.state.platform.systemRelease);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &instance.state.platform.systemVersion);
    yyjson_mut_obj_add_strbuf(doc, obj, "displayVersion", &instance.state.platform.systemDisplayVersion);
}
