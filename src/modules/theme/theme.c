#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/theme/theme.h"
#include "modules/theme/theme.h"
#include "util/stringUtils.h"

#define FF_THEME_NUM_FORMAT_ARGS 1

void ffPrintTheme(FFThemeOptions* options)
{
    FF_STRBUF_AUTO_DESTROY theme = ffStrbufCreate();
    const char* error = ffDetectTheme(&theme);

    if(error)
    {
        ffPrintError(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&theme, stdout);
    }
    else
    {
        ffPrintFormat(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_THEME_NUM_FORMAT_ARGS, (FFformatarg[]){
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

void ffParseThemeJsonObject(yyjson_val* module)
{
    FFThemeOptions __attribute__((__cleanup__(ffDestroyThemeOptions))) options;
    ffInitThemeOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(FF_THEME_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintTheme(&options);
}
