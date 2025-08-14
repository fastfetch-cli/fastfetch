#pragma once

#include "fastfetch.h"
#include "modules/font/option.h"

enum { FF_DETECT_FONT_NUM_FONTS = 4 };

typedef struct FFFontResult
{
    /**
     * Linux / BSD: Qt,      GTK2,  GTK3,        GTK4
     * MacOS:       System,  User,  System Mono, User Mono
     * Windows:     Caption, Menu,  Message,     Status
     * Haiku:       Plain,   Menu,  Bold,        Mono
     * Other:       Unset,   Unset, Unset,       Unset
     */
    FFstrbuf fonts[FF_DETECT_FONT_NUM_FONTS];
    FFstrbuf display;
} FFFontResult;

const char* ffDetectFont(FFFontResult* font);
