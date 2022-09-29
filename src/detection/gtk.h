#pragma once

#ifndef FF_INCLUDED_detection_gtk
#define FF_INCLUDED_detection_gtk

#include "fastfetch.h"

typedef struct FFGTKResult
{
    FFstrbuf theme;
    FFstrbuf icons;
    FFstrbuf font;
    FFstrbuf cursor;
    FFstrbuf cursorSize;
} FFGTKResult;

const FFGTKResult* ffDetectGTK2(const FFinstance* instance);
const FFGTKResult* ffDetectGTK4(const FFinstance* instance);
const FFGTKResult* ffDetectGTK3(const FFinstance* instance);

#endif
