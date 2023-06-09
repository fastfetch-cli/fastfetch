#include "fastfetch.h"
#include "common/printing.h"
#include "detection/theme/theme.h"
#include "modules/theme/theme.h"

#define FF_THEME_NUM_FORMAT_ARGS 1

void ffPrintTheme(FFinstance* instance, FFThemeOptions* options)
{
    FF_STRBUF_AUTO_DESTROY theme = ffStrbufCreate();
    const char* error = ffDetectTheme(instance, &theme);

    if(error)
    {
        ffPrintError(instance, FF_THEME_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_THEME_MODULE_NAME, 0, &options->moduleArgs.key);
        ffStrbufPutTo(&theme, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_THEME_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &theme}
        });
    }
}

void ffInitThemeOptions(FFThemeOptions* options)
{
    options->moduleName = FF_THEME_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseThemeCommandOptions(FFThemeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_THEME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyThemeOptions(FFThemeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseThemeJsonObject(FFinstance* instance, json_object* module)
{
    FFThemeOptions __attribute__((__cleanup__(ffDestroyThemeOptions))) options;
    ffInitThemeOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_THEME_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintTheme(instance, &options);
}
#endif
