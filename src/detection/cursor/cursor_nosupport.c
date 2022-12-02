#include "cursor.h"

void ffDetectCursor(const FFinstance* instance, FFCursorResult* result)
{
    FF_UNUSED(instance, result);
    ffStrbufInitS(&result->error, "Not supported on this platform");
}
