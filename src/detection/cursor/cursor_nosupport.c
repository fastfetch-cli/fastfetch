#include "cursor.h"

void ffDetectCursor(FF_MAYBE_UNUSED FFCursorResult* result)
{
    ffStrbufInitS(&result->error, "Not supported on this platform");
}
