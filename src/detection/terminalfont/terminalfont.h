#pragma once

#ifndef FF_INCLUDED_detection_terminalfont_terminalfont
#define FF_INCLUDED_detection_terminalfont_terminalfont

#include "fastfetch.h"
#include "common/font.h"

typedef struct FFTerminalFontResult
{
    FFstrbuf error;
    FFfont font;
} FFTerminalFontResult;

const FFTerminalFontResult* ffDetectTerminalFont(const FFinstance* instance);

#endif
