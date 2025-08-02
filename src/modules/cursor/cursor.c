#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/cursor/cursor.h"
#include "modules/cursor/cursor.h"
#include "util/stringUtils.h"

void ffPrintCursor(FFCursorOptions* options)
{
    FFCursorResult result;
    ffStrbufInit(&result.error);
    ffStrbufInit(&result.theme);
    ffStrbufInit(&result.size);

    ffDetectCursor(&result);

    if(result.error.length)
        ffPrintError(FF_CURSOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", result.error.chars);
    else
    {
        ffStrbufRemoveIgnCaseEndS(&result.theme, "cursors");
        ffStrbufRemoveIgnCaseEndS(&result.theme, "cursor");
        ffStrbufTrimRight(&result.theme, '_');
        ffStrbufTrimRight(&result.theme, '-');
        if(result.theme.length == 0)
            ffStrbufAppendS(&result.theme, "default");

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_CURSOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            ffStrbufWriteTo(&result.theme, stdout);

            if(result.size.length > 0 && !ffStrbufEqualS(&result.size, "0"))
                printf(" (%spx)", result.size.chars);

            putchar('\n');
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_CURSOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                FF_FORMAT_ARG(result.theme, "theme"),
                FF_FORMAT_ARG(result.size, "size"),
            }));
        }
    }

    ffStrbufDestroy(&result.error);
    ffStrbufDestroy(&result.theme);
    ffStrbufDestroy(&result.size);
}

bool ffParseCursorCommandOptions(FFCursorOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CURSOR_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseCursorJsonObject(FFCursorOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_CURSOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateCursorJsonConfig(FFCursorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyCursorOptions))) FFCursorOptions defaultOptions;
    ffInitCursorOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateCursorJsonResult(FF_MAYBE_UNUSED FFCursorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFCursorResult result;
    ffStrbufInit(&result.error);
    ffStrbufInit(&result.theme);
    ffStrbufInit(&result.size);

    ffDetectCursor(&result);

    if (result.error.length)
    {
        yyjson_mut_obj_add_strbuf(doc, module, "error", &result.error);
    }
    else
    {
        yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
        yyjson_mut_obj_add_strbuf(doc, obj, "theme", &result.theme);
        yyjson_mut_obj_add_strbuf(doc, obj, "size", &result.size);
    }

    ffStrbufDestroy(&result.error);
    ffStrbufDestroy(&result.theme);
    ffStrbufDestroy(&result.size);
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_CURSOR_MODULE_NAME,
    .description = "Print cursor style name",
    .parseCommandOptions = (void*) ffParseCursorCommandOptions,
    .parseJsonObject = (void*) ffParseCursorJsonObject,
    .printModule = (void*) ffPrintCursor,
    .generateJsonResult = (void*) ffGenerateCursorJsonResult,
    .generateJsonConfig = (void*) ffGenerateCursorJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Cursor theme", "theme"},
        {"Cursor size", "size"},
    })),
};

void ffInitCursorOptions(FFCursorOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "󰆿");
}

void ffDestroyCursorOptions(FFCursorOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
