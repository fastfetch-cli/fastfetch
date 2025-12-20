#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/wmtheme/wmtheme.h"
#include "modules/wmtheme/wmtheme.h"
#include "util/stringUtils.h"

#define FF_WMTHEME_DISPLAY_NAME "WM Theme"

bool ffPrintWMTheme(FFWMThemeOptions* options)
{
    FF_STRBUF_AUTO_DESTROY themeOrError = ffStrbufCreate();
    if(!ffDetectWmTheme(&themeOrError))
    {
        ffPrintError(FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", themeOrError.chars);
        return false;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        puts(themeOrError.chars);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(themeOrError, "result"),
        }));
    }

    return true;
}

void ffParseWMThemeJsonObject(FFWMThemeOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_WMTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateWMThemeJsonConfig(FFWMThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateWMThemeJsonResult(FF_MAYBE_UNUSED FFWMThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY themeOrError = ffStrbufCreate();
    if(!ffDetectWmTheme(&themeOrError))
    {
        yyjson_mut_obj_add_strbuf(doc, module, "error", &themeOrError);
        return false;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &themeOrError);
    return true;
}

void ffInitWMThemeOptions(FFWMThemeOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰓸");
}

void ffDestroyWMThemeOptions(FFWMThemeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffWMThemeModuleInfo = {
    .name = FF_WMTHEME_MODULE_NAME,
    .description = "Print current theme of window manager",
    .initOptions = (void*) ffInitWMThemeOptions,
    .destroyOptions = (void*) ffDestroyWMThemeOptions,
    .parseJsonObject = (void*) ffParseWMThemeJsonObject,
    .printModule = (void*) ffPrintWMTheme,
    .generateJsonResult = (void*) ffGenerateWMThemeJsonResult,
    .generateJsonConfig = (void*) ffGenerateWMThemeJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"WM theme", "result"},
    }))
};
