#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/kernel/kernel.h"
#include "util/stringUtils.h"

#define FF_KERNEL_NUM_FORMAT_ARGS 6

void ffPrintKernel(FFKernelOptions* options)
{
    const FFPlatformSysinfo* info = &instance.state.platform.sysinfo;
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        printf("%s %s", info->name.chars, info->release.chars);

        if(info->displayVersion.length > 0)
            printf(" (%s)\n", info->displayVersion.chars);
        else
            putchar('\n');
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
        ffParseSize(info->pageSize, &str);
        FF_PRINT_FORMAT_CHECKED(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_KERNEL_NUM_FORMAT_ARGS, ((FFformatarg[]){
            FF_FORMAT_ARG(info->name, "sysname"),
            FF_FORMAT_ARG(info->release, "release"),
            FF_FORMAT_ARG(info->version, "version"),
            FF_FORMAT_ARG(info->architecture, "arch"),
            FF_FORMAT_ARG(info->displayVersion, "display-version"),
            FF_FORMAT_ARG(str, "page-size"),
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
    const FFPlatformSysinfo* info = &instance.state.platform.sysinfo;

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "architecture", &info->architecture);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &info->name);
    yyjson_mut_obj_add_strbuf(doc, obj, "release", &info->release);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &info->version);
    yyjson_mut_obj_add_strbuf(doc, obj, "displayVersion", &info->displayVersion);
    yyjson_mut_obj_add_uint(doc, obj, "pageSize", info->pageSize);
}

void ffPrintKernelHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_KERNEL_MODULE_NAME, "{1} {2}", FF_KERNEL_NUM_FORMAT_ARGS, ((const char* []) {
        "Sysname - sysname",
        "Release - release",
        "Version - version",
        "Architecture - arch",
        "Display version - display-version",
        "Page size - page-size",
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
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyKernelOptions(FFKernelOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
