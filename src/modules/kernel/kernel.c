#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/size.h"
#include "modules/kernel/kernel.h"
#include "util/stringUtils.h"

bool ffPrintKernel(FFKernelOptions* options)
{
    const FFPlatformSysinfo* info = &instance.state.platform.sysinfo;
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        printf("%s %s\n", info->name.chars, info->release.chars);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
        ffSizeAppendNum(info->pageSize, &str);
        FF_PRINT_FORMAT_CHECKED(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(info->name, "sysname"),
            FF_FORMAT_ARG(info->release, "release"),
            FF_FORMAT_ARG(info->version, "version"),
            FF_FORMAT_ARG(info->architecture, "arch"),
            FF_FORMAT_ARG(str, "page-size"),
        }));
    }

    return true;
}

void ffParseKernelJsonObject(FFKernelOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateKernelJsonConfig(FFKernelOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateKernelJsonResult(FF_MAYBE_UNUSED FFKernelOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFPlatformSysinfo* info = &instance.state.platform.sysinfo;

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "architecture", &info->architecture);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &info->name);
    yyjson_mut_obj_add_strbuf(doc, obj, "release", &info->release);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &info->version);
    yyjson_mut_obj_add_uint(doc, obj, "pageSize", info->pageSize);

    return true;
}

void ffInitKernelOptions(FFKernelOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyKernelOptions(FFKernelOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffKernelModuleInfo = {
    .name = FF_KERNEL_MODULE_NAME,
    .description = "Print system kernel version",
    .initOptions = (void*) ffInitKernelOptions,
    .destroyOptions = (void*) ffDestroyKernelOptions,
    .parseJsonObject = (void*) ffParseKernelJsonObject,
    .printModule = (void*) ffPrintKernel,
    .generateJsonResult = (void*) ffGenerateKernelJsonResult,
    .generateJsonConfig = (void*) ffGenerateKernelJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Sysname", "sysname"},
        {"Release", "release"},
        {"Version", "version"},
        {"Architecture", "arch"},
        {"Display version", "display-version"},
        {"Page size", "page-size"},
    }))
};
