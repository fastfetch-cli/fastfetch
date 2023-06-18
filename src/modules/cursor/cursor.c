#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/cursor/cursor.h"
#include "modules/cursor/cursor.h"

#define FF_CURSOR_NUM_FORMAT_ARGS 2

void ffPrintCursor(FFinstance* instance, FFCursorOptions* options)
{
    FFCursorResult result;
    ffStrbufInit(&result.error);
    ffStrbufInit(&result.theme);
    ffStrbufInit(&result.size);

    ffDetectCursor(instance, &result);

    if(result.error.length)
        ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &options->moduleArgs, "%s", result.error.chars);
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
            ffPrintLogoAndKey(instance, FF_CURSOR_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
            ffStrbufWriteTo(&result.theme, stdout);

            if(result.size.length > 0)
                printf(" (%spx)", result.size.chars);

            putchar('\n');
        }
        else
        {
            ffPrintFormat(instance, FF_CURSOR_MODULE_NAME, 0, &options->moduleArgs, FF_CURSOR_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.theme},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result.size}
            });
        }
    }

    ffStrbufDestroy(&result.error);
    ffStrbufDestroy(&result.theme);
    ffStrbufDestroy(&result.size);
}

void ffInitCursorOptions(FFCursorOptions* options)
{
    options->moduleName = FF_CURSOR_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseCursorCommandOptions(FFCursorOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CURSOR_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyCursorOptions(FFCursorOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseCursorJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFCursorOptions __attribute__((__cleanup__(ffDestroyCursorOptions))) options;
    ffInitCursorOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCursor(instance, &options);
}
