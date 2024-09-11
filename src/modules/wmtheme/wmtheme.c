#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/wmtheme/wmtheme.h"
#include "modules/wmtheme/wmtheme.h"
#include "util/stringUtils.h"

#define FF_WMTHEME_DISPLAY_NAME "WM Theme"
#define FF_WMTHEME_NUM_FORMAT_ARGS 1

void ffPrintWMTheme(FFWMThemeOptions* options)
{
    FF_STRBUF_AUTO_DESTROY themeOrError = ffStrbufCreate();
    if(ffDetectWmTheme(&themeOrError))
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            puts(themeOrError.chars);
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_WMTHEME_NUM_FORMAT_ARGS, ((FFformatarg[]){
                FF_FORMAT_ARG(themeOrError, "result"),
            }));
        }
    }
    else
    {
        ffPrintError(FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%*s", themeOrError.length, themeOrError.chars);
    }
}

bool ffParseWMThemeCommandOptions(FFWMThemeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WMTHEME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseWMThemeJsonObject(FFWMThemeOptions* options, yyjson_val* module)
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

        ffPrintError(FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateWMThemeJsonConfig(FFWMThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyWMThemeOptions))) FFWMThemeOptions defaultOptions;
    ffInitWMThemeOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateWMThemeJsonResult(FF_MAYBE_UNUSED FFWMThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY themeOrError = ffStrbufCreate();
    if(!ffDetectWmTheme(&themeOrError))
    {
        yyjson_mut_obj_add_strbuf(doc, module, "error", &themeOrError);
        return;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &themeOrError);
}

void ffPrintWMthemeHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_WMTHEME_MODULE_NAME, "{1}", FF_WMTHEME_NUM_FORMAT_ARGS, ((const char* []) {
        "WM theme - result",
    }));
}

void ffInitWMThemeOptions(FFWMThemeOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_WMTHEME_MODULE_NAME,
        "Print current theme of window manager",
        ffParseWMThemeCommandOptions,
        ffParseWMThemeJsonObject,
        ffPrintWMTheme,
        ffGenerateWMThemeJsonResult,
        ffPrintWMthemeHelpFormat,
        ffGenerateWMThemeJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰓸");
}

void ffDestroyWMThemeOptions(FFWMThemeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
