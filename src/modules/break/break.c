#include "common/printing.h"
#include "modules/break/break.h"

void ffPrintBreak(FFinstance* instance)
{
    ffLogoPrintLine(instance);
    putchar('\n');
}

void ffParseBreakJsonObject(FFinstance* instance, FF_MAYBE_UNUSED yyjson_val* module)
{
    return ffPrintBreak(instance);
}
