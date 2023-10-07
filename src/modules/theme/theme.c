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
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_THEME_MODULE_NAME, ffParseThemeCommandOptions, ffParseThemeJsonObject, ffPrintTheme, ffGenerateThemeJson, ffPrintThemeHelpFormat);
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

void ffParseThemeJsonObject(FFThemeOptions* options, yyjson_val* module)
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

        ffPrintError(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateThemeJson(FF_MAYBE_UNUSED FFThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY theme = ffStrbufCreate();
    const char* error = ffDetectTheme(&theme);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &theme);
}

void ffPrintThemeHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_THEME_MODULE_NAME, "{1}", FF_THEME_NUM_FORMAT_ARGS, (const char* []) {
        "Combined themes"
    });
}
