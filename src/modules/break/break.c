#include "fastfetch.h"
#include "common/printing.h"
#include "modules/break/break.h"

#define FF_BREAK_MODULE_NAME "Break"

void ffPrintBreak(FFinstance* instance)
{
    ffLogoPrintLine(instance);
    putchar('\n');
}

#ifdef FF_HAVE_JSONC
bool ffParseBreakJsonObject(FFinstance* instance, const char* type, FF_MAYBE_UNUSED json_object* module)
{
    if (strcasecmp(type, FF_BREAK_MODULE_NAME) != 0)
        return false;

    ffPrintBreak(instance);
    return true;
}
#endif
