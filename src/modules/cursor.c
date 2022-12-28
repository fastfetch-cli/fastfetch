#include "fastfetch.h"

#include "common/printing.h"
#include "detection/cursor/cursor.h"

#define FF_CURSOR_MODULE_NAME "Cursor"
#define FF_CURSOR_NUM_FORMAT_ARGS 2

static void printCursor(FFinstance* instance, FFCursorResult* cursor)
{
    ffStrbufRemoveIgnCaseEndS(&cursor->theme, "cursors");
    ffStrbufRemoveIgnCaseEndS(&cursor->theme, "cursor");
    ffStrbufTrimRight(&cursor->theme, '_');
    ffStrbufTrimRight(&cursor->theme, '-');
    if(cursor->theme.length == 0)
        ffStrbufAppendS(&cursor->theme, "default");

    if(instance->config.cursor.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursor.key);
        ffStrbufWriteTo(&cursor->theme, stdout);

        if(cursor->size.length > 0)
            printf(" (%spx)", cursor->size.chars);

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursor, FF_CURSOR_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &cursor->theme},
            {FF_FORMAT_ARG_TYPE_STRBUF, &cursor->size}
        });
    }
}


void ffPrintCursor(FFinstance* instance)
{
    FFCursorResult result;
    ffStrbufInit(&result.error);
    ffStrbufInit(&result.theme);
    ffStrbufInit(&result.size);

    ffDetectCursor(instance, &result);

    if(result.error.length)
        ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursor, "%s", result.error.chars);
    else
        printCursor(instance, &result);

    ffStrbufDestroy(&result.error);
    ffStrbufDestroy(&result.theme);
    ffStrbufDestroy(&result.size);
}
