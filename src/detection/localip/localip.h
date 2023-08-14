#pragma once

#ifndef FF_INCLUDED_detection_localip_localip
#define FF_INCLUDED_detection_localip_localip

#include "fastfetch.h"

typedef struct FFLocalIpResult
{
    FFstrbuf name;
    FFstrbuf ipv4;
    FFstrbuf ipv6;
    FFstrbuf mac;
    bool defaultRoute;

    #ifdef _WIN32
    uint32_t ifIndex;
    #endif
} FFLocalIpResult;

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results);

#endif
