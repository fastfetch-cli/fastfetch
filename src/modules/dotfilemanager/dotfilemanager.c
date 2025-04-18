#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/libc/libc.h"
#include "detection/dotfilemanager/dotfilemanager.h"
#include "modules/dotfilemanager/dotfilemanager.h"
#include "util/stringUtils.h"

void ffPrintDotfileManager(FFDotfileManagerOptions* options)
{
    FFDotfileManagerResult result = {
        .name = ffStrbufCreate(),
    };
    const char* error = ffDetectDotfileManager(&result);

    if (error)
    {
        ffPrintError(FF_DOTFILEMANAGER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_DOTFILEMANAGER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result.name, stdout);
        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_DOTFILEMANAGER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result.name, "name"),
        }));
    }

    ffStrbufDestroy(&result.name);
}

bool ffParseDotfileManagerCommandOptions(FFDotfileManagerOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DOTFILEMANAGER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseDotfileManagerJsonObject(FFDotfileManagerOptions* options, yyjson_val* module)
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

        ffPrintError(FF_DOTFILEMANAGER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateDotfileManagerJsonConfig(FFDotfileManagerOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyDotfileManagerOptions))) FFDotfileManagerOptions defaultOptions;
    ffInitDotfileManagerOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateDotfileManagerJsonResult(FF_MAYBE_UNUSED FFDotfileManagerOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFDotfileManagerResult result = {
        .name = ffStrbufCreate(),
    };

    const char* error = ffDetectDotfileManager(&result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &result.name);

    ffStrbufDestroy(&result.name);
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_DOTFILEMANAGER_MODULE_NAME,
    .description = "Print information about the dotfile manager",
    .parseCommandOptions = (void*) ffParseDotfileManagerCommandOptions,
    .parseJsonObject = (void*) ffParseDotfileManagerJsonObject,
    .printModule = (void*) ffPrintDotfileManager,
    .generateJsonResult = (void*) ffGenerateDotfileManagerJsonResult,
    .generateJsonConfig = (void*) ffGenerateDotfileManagerJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Name", "name"},
    }))
};

void ffInitDotfileManagerOptions(FFDotfileManagerOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "󰣫");
}

void ffDestroyDotfileManagerOptions(FFDotfileManagerOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
