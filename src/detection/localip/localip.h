#pragma once

#ifndef FF_INCLUDED_detection_localip_localip
#define FF_INCLUDED_detection_localip_localip

#include "fastfetch.h"

typedef struct FFLocalIpIoCounters
{
    uint64_t txBytes;
    uint64_t rxBytes;
    uint64_t txPackets;
    uint64_t rxPackets;
    uint64_t rxErrors;
    uint64_t txErrors;
    uint64_t rxDrops;
    uint64_t txDrops;
} FFLocalIpIoCounters;

typedef struct FFLocalIpResult
{
    FFstrbuf name;
    FFstrbuf ipv4;
    FFstrbuf ipv6;
    FFstrbuf mac;
    bool defaultRoute;

    FFLocalIpIoCounters ioCounters;

    #ifdef _WIN32
    uint32_t ifIndex;
    #endif
} FFLocalIpResult;

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results);

#endif
