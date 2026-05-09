#pragma once

#include "fastfetch.h"
#include "modules/wifi/option.h"

struct FFWifiInterface {
    FFstrbuf description;
    FFstrbuf status;
};

struct FFWifiConnection {
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

typedef struct FFWifiResult {
    struct FFWifiInterface inf;
    struct FFWifiConnection conn;
} FFWifiResult;

const char* ffDetectWifi(FFlist* result /*list of FFWifiItem*/);

static inline uint16_t ffWifiFreqToChannel(uint16_t frequency) {
    // Return 0 for unknown / non-standard frequencies.
    if (frequency == 2484) {
        return 14;
    }

    // 2.4 GHz channels 1-13
    if (frequency >= 2412 && frequency <= 2472 && ((frequency - 2407) % 5) == 0) {
        return (uint16_t) ((frequency - 2407) / 5);
    }

    // 4.9 GHz public safety band (e.g. channels 182-196)
    if (frequency >= 4910 && frequency <= 4980 && ((frequency - 4000) % 5) == 0) {
        return (uint16_t) ((frequency - 4000) / 5);
    }

    // 5 GHz channels
    if (frequency >= 5000 && frequency <= 5895 && ((frequency - 5000) % 5) == 0) {
        return (uint16_t) ((frequency - 5000) / 5);
    }

    // 6 GHz channels (Wi-Fi 6E/7)
    // 5935 MHz is a special case mapped to channel 2.
    if (frequency == 5935) {
        return 2;
    }
    if (frequency >= 5955 && frequency <= 7115 && ((frequency - 5950) % 5) == 0) {
        return (uint16_t) ((frequency - 5950) / 5);
    }

    return 0;
}
