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
        ffPrintError(FF_TERMINALSIZE_DISPLAY_NAME, 0, &options->moduleArgs, "Failed to detect terminal size");
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
            ffPrintFormat(FF_TERMINALSIZE_DISPLAY_NAME, 0, &options->moduleArgs, FF_TERMINALSIZE_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_UINT16, &result.rows},
                {FF_FORMAT_ARG_TYPE_UINT16, &result.columns},
                {FF_FORMAT_ARG_TYPE_UINT16, &result.width},
                {FF_FORMAT_ARG_TYPE_UINT16, &result.height}
            });
        }
    }
}

void ffInitTerminalSizeOptions(FFTerminalSizeOptions* options)
{
    options->moduleName = FF_TERMINALSIZE_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseTerminalSizeCommandOptions(FFTerminalSizeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TERMINALSIZE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyTerminalSizeOptions(FFTerminalSizeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseTerminalSizeJsonObject(yyjson_val* module)
{
    FFTerminalSizeOptions __attribute__((__cleanup__(ffDestroyTerminalSizeOptions))) options;
    ffInitTerminalSizeOptions(&options);

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

            ffPrintError(FF_TERMINALSIZE_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintTerminalSize(&options);
}
