#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/kernel/kernel.h"
#include "util/stringUtils.h"

#define FF_KERNEL_NUM_FORMAT_ARGS 5

void ffPrintKernel(FFKernelOptions* options)
{
    const FFPlatform* platform = &instance.state.platform;
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        printf("%s %s", platform->systemName.chars, platform->systemRelease.chars);

        if(platform->systemDisplayVersion.length > 0)
            printf(" (%s)\n", platform->systemDisplayVersion.chars);
        else
            putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_KERNEL_NUM_FORMAT_ARGS, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemName, "sysname"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemRelease, "release"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemVersion, "version"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemArchitecture, "arch"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &platform->systemDisplayVersion, "display-version"},
        }));
    }
}

bool ffParseKernelCommandOptions(FFKernelOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_KERNEL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
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

        ffPrintError(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateKernelJsonConfig(FFKernelOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyKernelOptions))) FFKernelOptions defaultOptions;
    ffInitKernelOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateKernelJsonResult(FF_MAYBE_UNUSED FFKernelOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "architecture", &instance.state.platform.systemArchitecture);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &instance.state.platform.systemName);
    yyjson_mut_obj_add_strbuf(doc, obj, "release", &instance.state.platform.systemRelease);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &instance.state.platform.systemVersion);
    yyjson_mut_obj_add_strbuf(doc, obj, "displayVersion", &instance.state.platform.systemDisplayVersion);
}

void ffPrintKernelHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_KERNEL_MODULE_NAME, "{1} {2}", FF_KERNEL_NUM_FORMAT_ARGS, ((const char* []) {
        "Sysname - sysname",
        "Release - release",
        "Version - version",
        "Architecture - arch",
        "Display version - display-version",
    }));
}

void ffInitKernelOptions(FFKernelOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_KERNEL_MODULE_NAME,
        "Print system kernel version",
        ffParseKernelCommandOptions,
        ffParseKernelJsonObject,
        ffPrintKernel,
        ffGenerateKernelJsonResult,
        ffPrintKernelHelpFormat,
        ffGenerateKernelJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyKernelOptions(FFKernelOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
