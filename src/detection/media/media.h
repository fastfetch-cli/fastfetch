#pragma once

#ifndef FF_INCLUDED_detection_media
#define FF_INCLUDED_detection_media

#include "fastfetch.h"

typedef struct FFMediaResult
{
    FFstrbuf busNameShort; //e.g. plasma-browser-integration
    FFstrbuf player; // e.g. Google Chrome
    FFstrbuf song;
    FFstrbuf artist;
    FFstrbuf album;
    FFstrbuf url;
    const char* error;
} FFMediaResult;

const FFMediaResult* ffDetectMedia(FFinstance* instance);

#endif
