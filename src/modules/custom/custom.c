#include "fastfetch.h"
#include "common/printing.h"
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

#ifdef FF_HAVE_JSONC
void ffParseCustomJsonObject(FFinstance* instance, json_object* module)
{
    FFCustomOptions __attribute__((__cleanup__(ffDestroyCustomOptions))) options;
    ffInitCustomOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_CUSTOM_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCustom(instance, &options);
}
#endif
