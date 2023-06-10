#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/custom/custom.h"
#include "util/textModifier.h"

void ffPrintCustom(FFinstance* instance, FFCustomOptions* options)
{
    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintError(instance, FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs, "output format must be set for custom module");
        return;
    }

    ffPrintLogoAndKey(instance, options->moduleArgs.key.length == 0 ? NULL : FF_CUSTOM_MODULE_NAME, 0, &options->moduleArgs.key);
    ffPrintUserString(options->moduleArgs.outputFormat.chars);
    puts(FASTFETCH_TEXT_MODIFIER_RESET);
}

void ffInitCustomOptions(FFCustomOptions* options)
{
    options->moduleName = FF_CUSTOM_MODULE_NAME;
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

void ffParseCustomJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFCustomOptions __attribute__((__cleanup__(ffDestroyCustomOptions))) options;
    ffInitCustomOptions(&options);

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

            ffPrintError(instance, FF_CUSTOM_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCustom(instance, &options);
}
