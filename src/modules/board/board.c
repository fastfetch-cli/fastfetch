#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/board/board.h"
#include "modules/board/board.h"
#include "util/stringUtils.h"

#define FF_BOARD_NUM_FORMAT_ARGS 3

void ffPrintBoard(FFinstance* instance, FFBoardOptions* options)
{
    FFBoardResult result;
    ffDetectBoard(&result);

    if(result.error.length > 0)
    {
        ffPrintError(instance, FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, "%*s", result.error.length, result.error.chars);
        goto exit;
    }

    if(result.boardName.length == 0)
    {
        ffPrintError(instance, FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, "board_name is not set.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BOARD_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        ffStrbufWriteTo(&result.boardName, stdout);
        if (result.boardVersion.length)
            printf(" (%s)", result.boardVersion.chars);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, FF_BOARD_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.boardName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.boardVendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.boardVersion},
        });
    }

exit:
    ffStrbufDestroy(&result.boardName);
    ffStrbufDestroy(&result.boardVendor);
    ffStrbufDestroy(&result.boardVersion);
    ffStrbufDestroy(&result.error);
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

void ffParseBoardJsonObject(FFinstance* instance, yyjson_val* module)
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

            ffPrintError(instance, FF_BOARD_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintBoard(instance, &options);
}
