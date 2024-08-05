#pragma once

#include "fastfetch.h"

struct FFWifiInterface
{
    FFstrbuf description;
    FFstrbuf status;
};

struct FFWifiConnection
{
    FFstrbuf status;
    FFstrbuf ssid;
    FFstrbuf bssid;
    FFstrbuf protocol;
    FFstrbuf security;
    double signalQuality; // Percentage
    double rxRate;
    double txRate;
};

typedef struct FFWifiResult
{
    struct FFWifiInterface inf;
    struct FFWifiConnection conn;
} FFWifiResult;

const char* ffDetectWifi(FFlist* result /*list of FFWifiItem*/);
