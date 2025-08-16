#pragma once

#include "fastfetch.h"
#include "modules/wifi/option.h"

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
    uint16_t channel;
    uint16_t frequency; // MHz
};

typedef struct FFWifiResult
{
    struct FFWifiInterface inf;
    struct FFWifiConnection conn;
} FFWifiResult;

const char* ffDetectWifi(FFlist* result /*list of FFWifiItem*/);

static inline uint16_t ffWifiFreqToChannel(uint16_t frequency)
{
    // https://github.com/opetryna/win32wifi/blob/master/win32wifi/Win32Wifi.py#L140
    // FIXME: Does it work for 6 GHz?
    if (frequency == 2484)
        return 14;
    else if (frequency < 2484)
        return (uint16_t) ((frequency - 2407) / 5);
    else
        return (uint16_t) ((frequency / 5) - 1000);
}
