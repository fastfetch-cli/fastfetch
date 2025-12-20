#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/board/board.h"
#include "modules/board/board.h"
#include "util/stringUtils.h"

bool ffPrintBoard(FFBoardOptions* options)
{
    bool success = false;
    FFBoardResult result;
    ffStrbufInit(&result.name);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.serial);

    const char* error = ffDetectBoard(&result);
    if(error)
    {
        ffPrintError(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(result.name.length == 0)
    {
        ffPrintError(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "board_name is not set.");
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
        FF_PRINT_FORMAT_CHECKED(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result.name, "name"),
            FF_FORMAT_ARG(result.vendor, "vendor"),
            FF_FORMAT_ARG(result.version, "version"),
            FF_FORMAT_ARG(result.serial, "serial"),
        }));
    }
    success = true;

exit:
    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.serial);
    return success;
}

void ffParseBoardJsonObject(FFBoardOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_BOARD_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateBoardJsonConfig(FFBoardOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateBoardJsonResult(FF_MAYBE_UNUSED FFBoardOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFBoardResult board;
    ffStrbufInit(&board.name);
    ffStrbufInit(&board.vendor);
    ffStrbufInit(&board.version);
    ffStrbufInit(&board.serial);

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
    yyjson_mut_obj_add_strbuf(doc, obj, "serial", &board.serial);
    success = true;

exit:
    ffStrbufDestroy(&board.name);
    ffStrbufDestroy(&board.vendor);
    ffStrbufDestroy(&board.version);
    ffStrbufDestroy(&board.serial);
    return success;
}

void ffInitBoardOptions(FFBoardOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyBoardOptions(FFBoardOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffBoardModuleInfo = {
    .name = FF_BOARD_MODULE_NAME,
    .description = "Print motherboard name and other info",
    .initOptions = (void*) ffInitBoardOptions,
    .destroyOptions = (void*) ffDestroyBoardOptions,
    .parseJsonObject = (void*) ffParseBoardJsonObject,
    .printModule = (void*) ffPrintBoard,
    .generateJsonResult = (void*) ffGenerateBoardJsonResult,
    .generateJsonConfig = (void*) ffGenerateBoardJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Board name", "name"},
        {"Board vendor", "vendor"},
        {"Board version", "version"},
        {"Board serial number", "serial"},
    }))
};
