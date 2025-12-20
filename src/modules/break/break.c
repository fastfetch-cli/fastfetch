#include "common/printing.h"
#include "logo/logo.h"
#include "modules/break/break.h"

bool ffPrintBreak(FF_MAYBE_UNUSED FFBreakOptions* options)
{
    ffLogoPrintLine();
    putchar('\n');
    return true;
}

void ffParseBreakJsonObject(FF_MAYBE_UNUSED FFBreakOptions* options, FF_MAYBE_UNUSED yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (unsafe_yyjson_equals_str(key, "type") || unsafe_yyjson_equals_str(key, "condition"))
            continue;

        ffPrintError(FF_BREAK_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffInitBreakOptions(FF_MAYBE_UNUSED FFBreakOptions* options)
{
}

void ffDestroyBreakOptions(FF_MAYBE_UNUSED FFBreakOptions* options)
{
}

FFModuleBaseInfo ffBreakModuleInfo = {
    .name = FF_BREAK_MODULE_NAME,
    .description = "Print a empty line",
    .initOptions = (void*) ffInitBreakOptions,
    .destroyOptions = (void*) ffDestroyBreakOptions,
    .parseJsonObject = (void*) ffParseBreakJsonObject,
    .printModule = (void*) ffPrintBreak,
};
