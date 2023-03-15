#include "fastfetch.h"
#include "common/printing.h"
#include "modules/break/break.h"

void ffPrintBreak(FFinstance* instance)
{
    ffLogoPrintLine(instance);
    putchar('\n');
}

#ifdef FF_HAVE_JSONC
void ffParseBreakJsonObject(FFinstance* instance, FF_MAYBE_UNUSED json_object* module)
{
    return ffPrintBreak(instance);
}
#endif
