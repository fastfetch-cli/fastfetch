#pragma once

#include "fastfetch.h"

typedef struct FFLocalIpResult
{
    FFstrbuf name;
    FFstrbuf ipv4;
    FFstrbuf ipv6;
    FFstrbuf gateway4;
    FFstrbuf gateway6;
    FFstrbuf mac;
    bool defaultRoute;
} FFLocalIpResult;

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results);
