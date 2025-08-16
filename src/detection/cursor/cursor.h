#pragma once

#include "fastfetch.h"
#include "modules/cursor/option.h"

typedef struct FFCursorResult
{
    FFstrbuf theme;
    FFstrbuf size;
    FFstrbuf error;
} FFCursorResult;

void ffDetectCursor(FFCursorResult* result);
