#pragma once

#ifndef FF_INCLUDED_detection_network_network
#define FF_INCLUDED_detection_network_network

#include "fastfetch.h"

typedef struct FFNetworkResult
{
    FFstrbuf name;
    FFstrbuf type;
    FFstrbuf address;
    uint32_t mtu;
    bool up;
} FFNetworkResult;

const char* ffDetectNetwork(FFinstance* instance, FFlist* result);

#endif
