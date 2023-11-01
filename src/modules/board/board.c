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
        ffPrintLogoAndKey(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
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

bool ffParseBoardCommandOptions(FFBoardOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BOARD_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseBoardJsonObject(FFBoardOptions* options, yyjson_val* module)
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

        ffPrintError(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateBoardJsonConfig(FFBoardOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyBoardOptions))) FFBoardOptions defaultOptions;
    ffInitBoardOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateBoardJsonResult(FF_MAYBE_UNUSED FFBoardOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFBoardResult board;
    ffStrbufInit(&board.name);
    ffStrbufInit(&board.vendor);
    ffStrbufInit(&board.version);

    const char* error = ffDetectBoard(&board);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if (board.name.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "board_name is not set.");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &board.name);
    yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &board.vendor);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &board.version);

exit:
    ffStrbufDestroy(&board.name);
    ffStrbufDestroy(&board.vendor);
    ffStrbufDestroy(&board.version);
}

void ffPrintBoardHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_BOARD_MODULE_NAME, "{1} ({3})", FF_BOARD_NUM_FORMAT_ARGS, (const char* []) {
        "board name",
        "board vendor",
        "board version"
    });
}

void ffInitBoardOptions(FFBoardOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BOARD_MODULE_NAME,
        ffParseBoardCommandOptions,
        ffParseBoardJsonObject,
        ffPrintBoard,
        ffGenerateBoardJsonResult,
        ffPrintBoardHelpFormat,
        ffGenerateBoardJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyBoardOptions(FFBoardOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
