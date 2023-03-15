#include "fastfetch.h"
#include "common/printing.h"
#include "detection/board/board.h"
#include "modules/board/board.h"

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
        ffPrintLogoAndKey(instance, FF_BOARD_MODULE_NAME, 0, &options->moduleArgs.key);
        puts(result.boardName.chars);
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

#ifdef FF_HAVE_JSONC
void ffParseBoardJsonObject(FFinstance* instance, json_object* module)
{
    FFBoardOptions __attribute__((__cleanup__(ffDestroyBoardOptions))) options;
    ffInitBoardOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_BOARD_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintBoard(instance, &options);
}
#endif
