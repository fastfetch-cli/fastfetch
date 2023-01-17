#pragma once

#ifndef FF_INCLUDED_detection_displayserver
#define FF_INCLUDED_detection_displayserver

#include "fastfetch.h"

typedef struct FFDisplayResult
{
    uint32_t width;
    uint32_t height;
    uint32_t refreshRate;
    uint32_t scaledWidth;
    uint32_t scaledHeight;
} FFDisplayResult;

typedef struct FFDisplayServerResult
{
    FFstrbuf wmProcessName;
    FFstrbuf wmPrettyName;
    FFstrbuf wmProtocolName;
    FFstrbuf deProcessName;
    FFstrbuf dePrettyName;
    FFstrbuf deVersion;
    FFlist displays; //List of FFDisplayResult
} FFDisplayServerResult;

const FFDisplayServerResult* ffConnectDisplayServer(const FFinstance* instance);

//Used internal
uint32_t ffdsParseRefreshRate(int32_t refreshRate);
bool ffdsAppendDisplay(FFDisplayServerResult* result, uint32_t width, uint32_t height, uint32_t refreshRate, uint32_t scaledWidth, uint32_t scaledHeight);

#endif
