#pragma once

#ifndef FF_INCLUDED_detection_cursor_cursor
#define FF_INCLUDED_detection_cursor_cursor

#include "fastfetch.h"

typedef struct FFCursorResult
{
    FFstrbuf theme;
    FFstrbuf size;
    FFstrbuf error;
} FFCursorResult;

void ffDetectCursor(const FFinstance* instance, FFCursorResult* result);

#endif
