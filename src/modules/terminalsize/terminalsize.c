#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalsize/terminalsize.h"
#include "modules/terminalsize/terminalsize.h"
#include "util/stringUtils.h"

#define FF_TERMINALSIZE_DISPLAY_NAME "Terminal Size"
#define FF_TERMINALSIZE_NUM_FORMAT_ARGS 4

void ffPrintTerminalSize(FFTerminalSizeOptions* options)
{
    FFTerminalSizeResult result = {};

    if(!ffDetectTerminalSize(&result))
    {
        ffPrintError(FF_TERMINALSIZE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Failed to detect terminal size");
    }
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_TERMINALSIZE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            printf("%u columns x %u rows", result.columns, result.rows);

            if (result.width != 0 && result.height != 0)
                printf(" (%upx x %upx)", result.width, result.height);

            putchar('\n');
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_TERMINALSIZE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_TERMINALSIZE_NUM_FORMAT_ARGS, ((FFformatarg[]){
                FF_FORMAT_ARG(result.rows, "rows"),
                FF_FORMAT_ARG(result.columns, "columns"),
                FF_FORMAT_ARG(result.width, "width"),
                FF_FORMAT_ARG(result.height, "height"),
            }));
        }
    }
}

bool ffParseTerminalSizeCommandOptions(FFTerminalSizeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TERMINALSIZE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseTerminalSizeJsonObject(FFTerminalSizeOptions* options, yyjson_val* module)
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

        ffPrintError(FF_TERMINALSIZE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateTerminalSizeJsonConfig(FFTerminalSizeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyTerminalSizeOptions))) FFTerminalSizeOptions defaultOptions;
    ffInitTerminalSizeOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateTerminalSizeJsonResult(FF_MAYBE_UNUSED FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFTerminalSizeResult result;

    if(!ffDetectTerminalSize(&result))
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Failed to detect terminal size");
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_uint(doc, obj, "columns", result.columns);
    yyjson_mut_obj_add_uint(doc, obj, "rows", result.rows);
    yyjson_mut_obj_add_uint(doc, obj, "width", result.width);
    yyjson_mut_obj_add_uint(doc, obj, "height", result.height);
}

void ffPrintTerminalSizeHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_TERMINALSIZE_MODULE_NAME, "{1} columns x {2} rows ({3}px x {4}px)", FF_TERMINALSIZE_NUM_FORMAT_ARGS, ((const char* []) {
        "Terminal rows - rows",
        "Terminal columns - columns",
        "Terminal width (in pixels) - width",
        "Terminal height (in pixels) - height",
    }));
}

void ffInitTerminalSizeOptions(FFTerminalSizeOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_TERMINALSIZE_MODULE_NAME,
        "Print current terminal size",
        ffParseTerminalSizeCommandOptions,
        ffParseTerminalSizeJsonObject,
        ffPrintTerminalSize,
        ffGenerateTerminalSizeJsonResult,
        ffPrintTerminalSizeHelpFormat,
        ffGenerateTerminalSizeJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰲎");
}

void ffDestroyTerminalSizeOptions(FFTerminalSizeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
