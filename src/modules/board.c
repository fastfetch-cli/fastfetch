#include "fastfetch.h"
#include "common/printing.h"
#include "detection/board/board.h"

#define FF_BOARD_MODULE_NAME "Board"
#define FF_BOARD_NUM_FORMAT_ARGS 3

void ffPrintBoard(FFinstance* instance)
{
    FFBoardResult result;
    ffDetectBoard(&result);

    if(result.error.length > 0)
    {
        ffPrintError(instance, FF_BOARD_MODULE_NAME, 0, &instance->config.board, "%*s", result.error.length, result.error.chars);
        goto exit;
    }

    if(result.boardName.length == 0)
    {
        ffPrintError(instance, FF_BOARD_MODULE_NAME, 0, &instance->config.board, "board_name is not set.");
        goto exit;
    }

    if(instance->config.board.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BOARD_MODULE_NAME, 0, &instance->config.board.key);
        puts(result.boardName.chars);
    }
    else
    {
        ffPrintFormat(instance, FF_BOARD_MODULE_NAME, 0, &instance->config.board, FF_BOARD_NUM_FORMAT_ARGS, (FFformatarg[]) {
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
