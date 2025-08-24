#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/custom/custom.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

bool ffPrintCustom(FFCustomOptions* options)
{
    ffPrintFormat(FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, 0, ((FFformatarg[]) {}));
    return true;
}

void ffGenerateCustomJsonConfig(FFCustomOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

void ffParseCustomJsonObject(FFCustomOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffInitCustomOptions(FFCustomOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
    ffStrbufSetStatic(&options->moduleArgs.key, " ");
}

void ffDestroyCustomOptions(FFCustomOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffCustomModuleInfo = {
    .name = FF_CUSTOM_MODULE_NAME,
    .description = "Print a custom string, with or without key",
    .initOptions = (void*) ffInitCustomOptions,
    .destroyOptions = (void*) ffDestroyCustomOptions,
    .parseJsonObject = (void*) ffParseCustomJsonObject,
    .printModule = (void*) ffPrintCustom,
    .generateJsonConfig = (void*) ffGenerateCustomJsonConfig,
};
