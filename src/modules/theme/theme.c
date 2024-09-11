#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/theme/theme.h"
#include "modules/theme/theme.h"
#include "util/stringUtils.h"

#define FF_THEME_NUM_FORMAT_ARGS 2

void ffPrintTheme(FFThemeOptions* options)
{
    FFThemeResult result = {
        .theme1 = ffStrbufCreate(),
        .theme2 = ffStrbufCreate()
    };
    const char* error = ffDetectTheme(&result);

    if(error)
    {
        ffPrintError(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        if (result.theme1.length)
            ffStrbufWriteTo(&result.theme1, stdout);
        if (result.theme2.length)
        {
            if (result.theme1.length)
                fputs(", ", stdout);
            ffStrbufWriteTo(&result.theme2, stdout);
        }
        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_THEME_NUM_FORMAT_ARGS, ((FFformatarg[]){
            FF_FORMAT_ARG(result.theme1, "theme1"),
            FF_FORMAT_ARG(result.theme2, "theme2"),
        }));
    }

    ffStrbufDestroy(&result.theme1);
    ffStrbufDestroy(&result.theme2);
}

bool ffParseThemeCommandOptions(FFThemeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_THEME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
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

        ffPrintError(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateThemeJsonConfig(FFThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyThemeOptions))) FFThemeOptions defaultOptions;
    ffInitThemeOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateThemeJsonResult(FF_MAYBE_UNUSED FFThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFThemeResult result = {
        .theme1 = ffStrbufCreate(),
        .theme2 = ffStrbufCreate()
    };
    const char* error = ffDetectTheme(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* theme = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, theme, "theme1", &result.theme1);
    yyjson_mut_obj_add_strbuf(doc, theme, "theme2", &result.theme2);

    ffStrbufDestroy(&result.theme1);
    ffStrbufDestroy(&result.theme2);
}

void ffPrintThemeHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_THEME_MODULE_NAME, "{1}, {2}", FF_THEME_NUM_FORMAT_ARGS, ((const char* []) {
        "Theme part 1 - theme1",
        "Theme part 2 - theme2",
    }));
}

void ffInitThemeOptions(FFThemeOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_THEME_MODULE_NAME,
        "Print current theme of desktop environment",
        ffParseThemeCommandOptions,
        ffParseThemeJsonObject,
        ffPrintTheme,
        ffGenerateThemeJsonResult,
        ffPrintThemeHelpFormat,
        ffGenerateThemeJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰉼");
}

void ffDestroyThemeOptions(FFThemeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
