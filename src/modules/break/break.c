#include "common/printing.h"
#include "modules/break/break.h"

void ffPrintBreak(void)
{
    ffLogoPrintLine();
    putchar('\n');
}

void ffParseBreakJsonObject(FF_MAYBE_UNUSED yyjson_val* module)
{
    return ffPrintBreak();
}
