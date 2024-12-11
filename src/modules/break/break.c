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

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_BREAK_MODULE_NAME,
    .description = "Print a empty line",
    .parseCommandOptions = (void*) ffParseBreakCommandOptions,
    .parseJsonObject = (void*) ffParseBreakJsonObject,
    .printModule = (void*) ffPrintBreak,
};

void ffInitBreakOptions(FF_MAYBE_UNUSED FFBreakOptions* options)
{
    options->moduleInfo = ffModuleInfo;
}

void ffDestroyBreakOptions(FF_MAYBE_UNUSED FFBreakOptions* options)
{
}
