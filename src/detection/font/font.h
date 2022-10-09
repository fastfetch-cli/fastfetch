#pragma once

#ifndef FF_INCLUDED_detection_font_font
#define FF_INCLUDED_detection_font_font

#include "fastfetch.h"

#define FF_DETECT_FONT_NUM_FONTS 4

typedef struct FFFontResult
{
    FFstrbuf error;

    /**
     * Linux / BSD: QT,      GTK2,  GTK3,      GTK4
     * MacOS:       System,  User,  Monospace, Application
     * Windows:     Desktop, Unset, Unset,     Unset
     * Other:       Unset,   Unset, Unset,     Unset
     */
    FFstrbuf fonts[FF_DETECT_FONT_NUM_FONTS];
} FFFontResult;

const FFFontResult* ffDetectFont(const FFinstance* instance);

#endif
