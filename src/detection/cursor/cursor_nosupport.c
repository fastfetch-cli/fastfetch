#include "cursor.h"

void ffDetectCursor([[maybe_unused]] FFCursorResult* result) {
    ffStrbufInitS(&result->error, "Not supported on this platform");
}
