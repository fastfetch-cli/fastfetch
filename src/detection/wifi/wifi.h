#pragma once

#ifndef FF_INCLUDED_detection_wifi_wifi
#define FF_INCLUDED_detection_wifi_wifi

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
    FFstrbuf macAddress;
    FFstrbuf phyType;
    double signalQuality; // Percentage
    double rxRate;
    double txRate;
};

struct FFWifiSecurity
{
    bool enabled;
    bool oneXEnabled;
    FFstrbuf algorithm;
};

typedef struct FFWifiResult
{
    struct FFWifiInterface inf;
    struct FFWifiConnection conn;
    struct FFWifiSecurity security;
} FFWifiResult;

const char* ffDetectWifi(const FFinstance* instance, FFlist* result /*list of FFWifiItem*/);

#endif
