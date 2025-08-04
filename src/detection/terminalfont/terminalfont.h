#pragma once

#include "fastfetch.h"
#include "common/font.h"
#include "modules/font/option.h"

typedef struct FFTerminalFontResult
{
    FFstrbuf error;
    FFfont font;
    FFfont fallback;
} FFTerminalFontResult;

bool ffDetectTerminalFont(FFTerminalFontResult* result);
