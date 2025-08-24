#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalsize/terminalsize.h"
#include "modules/terminalsize/terminalsize.h"
#include "util/stringUtils.h"

#define FF_TERMINALSIZE_DISPLAY_NAME "Terminal Size"

bool ffPrintTerminalSize(FFTerminalSizeOptions* options)
{
    FFTerminalSizeResult result = {};

    if(!ffDetectTerminalSize(&result))
    {
        ffPrintError(FF_TERMINALSIZE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Failed to detect terminal size");
        return false;
    }

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
        FF_PRINT_FORMAT_CHECKED(FF_TERMINALSIZE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result.rows, "rows"),
            FF_FORMAT_ARG(result.columns, "columns"),
            FF_FORMAT_ARG(result.width, "width"),
            FF_FORMAT_ARG(result.height, "height"),
        }));
    }
    return true;
}

void ffParseTerminalSizeJsonObject(FFTerminalSizeOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_TERMINALSIZE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateTerminalSizeJsonConfig(FFTerminalSizeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateTerminalSizeJsonResult(FF_MAYBE_UNUSED FFTerminalSizeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFTerminalSizeResult result;

    if(!ffDetectTerminalSize(&result))
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Failed to detect terminal size");
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_uint(doc, obj, "columns", result.columns);
    yyjson_mut_obj_add_uint(doc, obj, "rows", result.rows);
    yyjson_mut_obj_add_uint(doc, obj, "width", result.width);
    yyjson_mut_obj_add_uint(doc, obj, "height", result.height);

    return true;
}

void ffInitTerminalSizeOptions(FFTerminalSizeOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰲎");
}

void ffDestroyTerminalSizeOptions(FFTerminalSizeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffTerminalSizeModuleInfo = {
    .name = FF_TERMINALSIZE_MODULE_NAME,
    .description = "Print current terminal size",
    .initOptions = (void*) ffInitTerminalSizeOptions,
    .destroyOptions = (void*) ffDestroyTerminalSizeOptions,
    .parseJsonObject = (void*) ffParseTerminalSizeJsonObject,
    .printModule = (void*) ffPrintTerminalSize,
    .generateJsonResult = (void*) ffGenerateTerminalSizeJsonResult,
    .generateJsonConfig = (void*) ffGenerateTerminalSizeJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Terminal rows", "rows"},
        {"Terminal columns", "columns"},
        {"Terminal width (in pixels)", "width"},
        {"Terminal height (in pixels)", "height"},
    })),
};
