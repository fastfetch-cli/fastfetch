#include "common/printing.h"
#include "modules/break/break.h"

void ffPrintBreak(FF_MAYBE_UNUSED FFBreakOptions* options)
{
    ffLogoPrintLine();
    putchar('\n');
}

bool ffParseBreakCommandOptions(FF_MAYBE_UNUSED FFBreakOptions* options, FF_MAYBE_UNUSED const char* key, FF_MAYBE_UNUSED const char* value)
{
    return false;
}

void ffParseBreakJsonObject(FF_MAYBE_UNUSED FFBreakOptions* options, FF_MAYBE_UNUSED yyjson_val* module)
{
}

void ffInitBreakOptions(FF_MAYBE_UNUSED FFBreakOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BREAK_MODULE_NAME,
        "Print a empty line",
        ffParseBreakCommandOptions,
        ffParseBreakJsonObject,
        ffPrintBreak,
        NULL,
        NULL,
        NULL
    );
}

void ffDestroyBreakOptions(FF_MAYBE_UNUSED FFBreakOptions* options)
{
}
