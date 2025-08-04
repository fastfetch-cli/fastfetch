#pragma once

#include "fastfetch.h"
#include "modules/battery/option.h"

#define FF_BATTERY_TEMP_UNSET (0/0.0)

typedef struct FFBatteryResult
{
    FFstrbuf manufacturer;
    FFstrbuf manufactureDate;
    FFstrbuf modelName;
    FFstrbuf technology;
    FFstrbuf status;
    FFstrbuf serial;
    double capacity;
    double temperature;
    uint32_t cycleCount;
    int32_t timeRemaining; // in seconds, -1 if unknown
} FFBatteryResult;

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results);
