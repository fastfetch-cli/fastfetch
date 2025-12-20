#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/theme/theme.h"
#include "modules/theme/theme.h"
#include "util/stringUtils.h"

bool ffPrintTheme(FFThemeOptions* options)
{
    FFThemeResult result = {
        .theme1 = ffStrbufCreate(),
        .theme2 = ffStrbufCreate()
    };
    const char* error = ffDetectTheme(&result);

    if(error)
    {
        ffPrintError(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
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
        FF_PRINT_FORMAT_CHECKED(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result.theme1, "theme1"),
            FF_FORMAT_ARG(result.theme2, "theme2"),
        }));
    }

    ffStrbufDestroy(&result.theme1);
    ffStrbufDestroy(&result.theme2);
    return true;
}

void ffParseThemeJsonObject(FFThemeOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_THEME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateThemeJsonConfig(FFThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateThemeJsonResult(FF_MAYBE_UNUSED FFThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFThemeResult result = {
        .theme1 = ffStrbufCreate(),
        .theme2 = ffStrbufCreate()
    };
    const char* error = ffDetectTheme(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* theme = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, theme, "theme1", &result.theme1);
    yyjson_mut_obj_add_strbuf(doc, theme, "theme2", &result.theme2);

    ffStrbufDestroy(&result.theme1);
    ffStrbufDestroy(&result.theme2);

    return true;
}

void ffInitThemeOptions(FFThemeOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰉼");
}

void ffDestroyThemeOptions(FFThemeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffThemeModuleInfo = {
    .name = FF_THEME_MODULE_NAME,
    .description = "Print current theme of desktop environment",
    .initOptions = (void*) ffInitThemeOptions,
    .destroyOptions = (void*) ffDestroyThemeOptions,
    .parseJsonObject = (void*) ffParseThemeJsonObject,
    .printModule = (void*) ffPrintTheme,
    .generateJsonResult = (void*) ffGenerateThemeJsonResult,
    .generateJsonConfig = (void*) ffGenerateThemeJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Theme part 1", "theme1"},
        {"Theme part 2", "theme2"},
    }))
};
