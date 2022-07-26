#pragma once

#ifndef FF_INCLUDED_detection_qt
#define FF_INCLUDED_detection_qt

#include "util/FFstrbuf.h"

typedef struct FFQtResult
{
    FFstrbuf widgetStyle;
    FFstrbuf colorScheme;
    FFstrbuf icons;
    FFstrbuf font;
} FFQtResult;

const FFQtResult* ffDetectQt(FFinstance* instance);

#endif
