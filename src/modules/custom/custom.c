#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/custom/custom.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

void ffPrintCustom(FFCustomOptions* options)
{
    ffPrintFormat(FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, 0, ((FFformatarg[]) {}));
}

bool ffParseCustomCommandOptions(FFCustomOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CUSTOM_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffGenerateCustomJsonConfig(FFCustomOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyCustomOptions))) FFCustomOptions defaultOptions;
    ffInitCustomOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffParseCustomJsonObject(FFCustomOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type") || ffStrEqualsIgnCase(key, "condition"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_CUSTOM_MODULE_NAME,
    .description = "Print a custom string, with or without key",
    .parseCommandOptions = (void*) ffParseCustomCommandOptions,
    .parseJsonObject = (void*) ffParseCustomJsonObject,
    .printModule = (void*) ffPrintCustom,
    .generateJsonConfig = (void*) ffGenerateCustomJsonConfig,
};

void ffInitCustomOptions(FFCustomOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "");
    ffStrbufSetStatic(&options->moduleArgs.key, " ");
}

void ffDestroyCustomOptions(FFCustomOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
