#pragma once

#ifndef FF_INCLUDED_detection_font_font
#define FF_INCLUDED_detection_font_font

#include "fastfetch.h"
#include "util/FFstrbuf.h"
#include "util/FFlist.h"

typedef struct FFFontResult
{
    const char* type;
    FFstrbuf fontPretty;
} FFFontResult;

const char* ffDetectFontImpl(FFinstance* instance, FFlist* result);

#endif
