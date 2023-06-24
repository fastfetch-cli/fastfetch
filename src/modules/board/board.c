#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/board/board.h"
#include "modules/board/board.h"
#include "util/stringUtils.h"

#define FF_BOARD_NUM_FORMAT_ARGS 3

void ffPrintBoard(FFBoardOptions* options)
{
    FFBoardResult result;
    ffStrbufInit(&result.name);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.version);
    const char* error = ffDetectBoard(&result);

    if(error)
    {
        ffPrintError(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        goto exit;
    }

    if(result.name.length == 0)
    {
        ffPrintError(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, "board_name is not set.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        ffStrbufWriteTo(&result.name, stdout);
        if (result.version.length)
            printf(" (%s)", result.version.chars);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, FF_BOARD_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.version},
        });
    }

exit:
    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.version);
}

void ffInitBoardOptions(FFBoardOptions* options)
{
    options->moduleName = FF_BOARD_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseBoardCommandOptions(FFBoardOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BOARD_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyBoardOptions(FFBoardOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseBoardJsonObject(yyjson_val* module)
{
    FFBoardOptions __attribute__((__cleanup__(ffDestroyBoardOptions))) options;
    ffInitBoardOptions(&options);

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

            ffPrintError(FF_BOARD_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintBoard(&options);
}
