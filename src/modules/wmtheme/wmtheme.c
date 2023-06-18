#include "common/printing.h"
#include "common/jsonconfig.h"
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
            ffPrintLogoAndKey(instance, FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
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

void ffParseWMThemeJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFWMThemeOptions __attribute__((__cleanup__(ffDestroyWMThemeOptions))) options;
    ffInitWMThemeOptions(&options);

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

            ffPrintError(instance, FF_WMTHEME_DISPLAY_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintWMTheme(instance, &options);
}
