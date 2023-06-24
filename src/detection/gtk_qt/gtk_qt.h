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
    FFstrbuf wallpaper;
} FFQtResult;

const FFGTKResult* ffDetectGTK2(void);
const FFGTKResult* ffDetectGTK4(void);
const FFGTKResult* ffDetectGTK3(void);
const FFQtResult* ffDetectQt(void);

#endif
