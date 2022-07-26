#pragma once

#ifndef FF_INCLUDED_detection_displayserver
#define FF_INCLUDED_detection_displayserver

#include "fastfetch.h"

typedef struct FFResolutionResult
{
    uint32_t width;
    uint32_t height;
    uint32_t refreshRate;
} FFResolutionResult;

typedef struct FFDisplayServerResult
{
    FFstrbuf wmProcessName;
    FFstrbuf wmPrettyName;
    FFstrbuf wmProtocolName;
    FFstrbuf deProcessName;
    FFstrbuf dePrettyName;
    FFstrbuf deVersion;
    FFlist resolutions; //List of FFResolutionResult
} FFDisplayServerResult;

const FFDisplayServerResult* ffConnectDisplayServer(const FFinstance* instance);

#endif

