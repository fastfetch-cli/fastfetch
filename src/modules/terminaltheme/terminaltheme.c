#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminaltheme/terminaltheme.h"
#include "modules/terminaltheme/terminaltheme.h"
#include "util/stringUtils.h"

#include <inttypes.h>

#define FF_TERMINALTHEME_DISPLAY_NAME "Terminal Theme"
#define FF_TERMINALTHEME_NUM_FORMAT_ARGS 4

void ffPrintTerminalTheme(FFTerminalThemeOptions* options)
{
    FFTerminalThemeResult result = {};

    if(!ffDetectTerminalTheme(&result))
    {
        ffPrintError(FF_TERMINALTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Failed to detect terminal theme");
    }
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_TERMINALTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            printf("#%02" PRIX16 "%02" PRIX16 "%02" PRIX16 " (FG) - #%02" PRIX16 "%02" PRIX16 "%02" PRIX16 " (BG) [%s]\n",
                result.fg.r, result.fg.g, result.fg.b,
                result.bg.r, result.bg.g, result.bg.b,
                result.bg.dark ? "Dark" : "Light");
        }
        else
        {
            char fg[32], bg[32];
            snprintf(fg, sizeof(fg), "#%02" PRIX16 "%02" PRIX16 "%02" PRIX16, result.fg.r, result.fg.g, result.fg.b);
            snprintf(bg, sizeof(bg), "#%02" PRIX16 "%02" PRIX16 "%02" PRIX16, result.bg.r, result.bg.g, result.bg.b);
            FF_PRINT_FORMAT_CHECKED(FF_TERMINALTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_TERMINALTHEME_NUM_FORMAT_ARGS, ((FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRING, fg, "fg-color"},
                {FF_FORMAT_ARG_TYPE_STRING, result.fg.dark ? "Dark" : "Light", "fg-type"},
                {FF_FORMAT_ARG_TYPE_STRING, bg, "bg-color"},
                {FF_FORMAT_ARG_TYPE_STRING, result.bg.dark ? "Dark" : "Light", "bg-type"},
            }));
        }
    }
}

bool ffParseTerminalThemeCommandOptions(FFTerminalThemeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TERMINALTHEME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseTerminalThemeJsonObject(FFTerminalThemeOptions* options, yyjson_val* module)
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

        ffPrintError(FF_TERMINALTHEME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateTerminalThemeJsonConfig(FFTerminalThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyTerminalThemeOptions))) FFTerminalThemeOptions defaultOptions;
    ffInitTerminalThemeOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateTerminalThemeJsonResult(FF_MAYBE_UNUSED FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFTerminalThemeResult result = {};

    if(!ffDetectTerminalTheme(&result))
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Failed to detect terminal theme");
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");

    yyjson_mut_val* fg = yyjson_mut_obj_add_obj(doc, obj, "fg");
    yyjson_mut_obj_add_uint(doc, fg, "r", result.fg.r);
    yyjson_mut_obj_add_uint(doc, fg, "g", result.fg.g);
    yyjson_mut_obj_add_uint(doc, fg, "b", result.fg.b);
    yyjson_mut_obj_add_bool(doc, fg, "dark", result.fg.dark);

    yyjson_mut_val* bg = yyjson_mut_obj_add_obj(doc, obj, "bg");
    yyjson_mut_obj_add_uint(doc, bg, "r", result.bg.r);
    yyjson_mut_obj_add_uint(doc, bg, "g", result.bg.g);
    yyjson_mut_obj_add_uint(doc, bg, "b", result.bg.b);
    yyjson_mut_obj_add_bool(doc, bg, "dark", result.bg.dark);
}

void ffPrintTerminalThemeHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_TERMINALTHEME_MODULE_NAME, "{1} (FG) {3} (BG) [{4}]", FF_TERMINALTHEME_NUM_FORMAT_ARGS, ((const char* []) {
        "Terminal foreground color - fg-color",
        "Terminal foreground type (Dark / Light) - fg-type",
        "Terminal background color - bg-color",
        "Terminal background type (Dark / Light) - bg-type",
    }));
}

void ffInitTerminalThemeOptions(FFTerminalThemeOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_TERMINALTHEME_MODULE_NAME,
        "Print current terminal theme (foreground and background colors)",
        ffParseTerminalThemeCommandOptions,
        ffParseTerminalThemeJsonObject,
        ffPrintTerminalTheme,
        ffGenerateTerminalThemeJsonResult,
        ffPrintTerminalThemeHelpFormat,
        ffGenerateTerminalThemeJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyTerminalThemeOptions(FFTerminalThemeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
