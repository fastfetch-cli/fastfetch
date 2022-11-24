#include "cursor.h"

void ffDetectCursor(const FFinstance* instance, FFCursorResult* result)
{
    ffStrbufInitS(&result->error, "Not supported on this platform");
}
