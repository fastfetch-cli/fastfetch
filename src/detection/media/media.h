#pragma once

#include "fastfetch.h"
#include "modules/media/option.h"

typedef struct FFMediaResult
{
    FFstrbuf error;
    FFstrbuf playerId; // Bus name on Linux, app bundle name on macOS. e.g. plasma-browser-integration
    FFstrbuf player; // e.g. Google Chrome
    FFstrbuf song;
    FFstrbuf artist;
    FFstrbuf album;
    FFstrbuf url;
    FFstrbuf status;
} FFMediaResult;

const FFMediaResult* ffDetectMedia();
