#include "fastfetch.h"
#include "common/printing.h"
#include "detection/wmtheme/wmtheme.h"
#include "modules/wmtheme/wmtheme.h"

#define FF_WMTHEME_DISPLAY_NAME "WM Theme"
#define FF_WMTHEME_NUM_FORMAT_ARGS 1

void ffPrintWMTheme(FFinstance* instance, FFWMThemeOptions* options)
{
    FF_STRBUF_AUTO_DESTROY themeOrError = ffStrbufCreate();
    if(ffDetectWmTheme(instance, &themeOrError))
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs.key);
            puts(themeOrError.chars);
        }
        else
        {
            ffPrintFormat(instance, FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_WMTHEME_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &themeOrError}
            });
        }
    }
    else
    {
        ffPrintError(instance, FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, "%*s", themeOrError.length, themeOrError.chars);
    }
}

void ffInitWMThemeOptions(FFWMThemeOptions* options)
{
    options->moduleName = FF_WMTHEME_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseWMThemeCommandOptions(FFWMThemeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WMTHEME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyWMThemeOptions(FFWMThemeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseWMThemeJsonObject(FFinstance* instance, json_object* module)
{
    FFWMThemeOptions __attribute__((__cleanup__(ffDestroyWMThemeOptions))) options;
    ffInitWMThemeOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_WMTHEME_DISPLAY_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintWMTheme(instance, &options);
}
#endif
