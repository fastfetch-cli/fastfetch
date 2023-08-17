#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/custom/custom.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

void ffPrintCustom(FFCustomOptions* options)
{
    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintError(FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs, "output format must be set for custom module");
        return;
    }

    ffPrintLogoAndKey(options->moduleArgs.key.length == 0 ? NULL : FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
    ffStrbufWriteTo(&options->moduleArgs.outputFormat, stdout);
    puts(FASTFETCH_TEXT_MODIFIER_RESET);
}

void ffInitCustomOptions(FFCustomOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_CUSTOM_MODULE_NAME, ffParseCustomCommandOptions, ffParseCustomJsonObject, ffPrintCustom);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseCustomCommandOptions(FFCustomOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CUSTOM_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyCustomOptions(FFCustomOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseCustomJsonObject(FFCustomOptions* options, yyjson_val* module)
{
    if (module)
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

            ffPrintError(FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
        }
    }
}
