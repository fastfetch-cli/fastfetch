#pragma once

#ifndef FF_INCLUDED_detection_gtk
#define FF_INCLUDED_detection_gtk

#include "util/FFstrbuf.h"

typedef struct FFGTKResult
{
    FFstrbuf theme;
    FFstrbuf icons;
    FFstrbuf font;
    FFstrbuf cursor;
    FFstrbuf cursorSize;
} FFGTKResult;

const FFGTKResult* ffDetectGTK2(FFinstance* instance);
const FFGTKResult* ffDetectGTK4(FFinstance* instance);
const FFGTKResult* ffDetectGTK3(FFinstance* instance);

#endif
