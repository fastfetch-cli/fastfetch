#pragma once

#ifndef FF_INCLUDED_detection_gtkqt_gtkqt
#define FF_INCLUDED_detection_gtkqt_gtkqt

#include "fastfetch.h"

typedef struct FFGTKResult
{
    FFstrbuf theme;
    FFstrbuf icons;
    FFstrbuf font;
    FFstrbuf cursor;
    FFstrbuf cursorSize;
    FFstrbuf wallpaper;
} FFGTKResult;

typedef struct FFQtResult
{
    FFstrbuf widgetStyle;
    FFstrbuf colorScheme;
    FFstrbuf icons;
    FFstrbuf font;
} FFQtResult;

const FFGTKResult* ffDetectGTK2(const FFinstance* instance);
const FFGTKResult* ffDetectGTK4(const FFinstance* instance);
const FFGTKResult* ffDetectGTK3(const FFinstance* instance);
const FFQtResult* ffDetectQt(const FFinstance* instance);

#endif
