#pragma once

#include "fastfetch.h"

#define FF_BATTERY_TEMP_UNSET (0/0.0)

typedef struct FFBatteryResult
{
    FFstrbuf manufacturer;
    FFstrbuf modelName;
    FFstrbuf technology;
    double capacity;
    FFstrbuf status;
    double temperature;
    uint32_t cycleCount;
} FFBatteryResult;

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results);
