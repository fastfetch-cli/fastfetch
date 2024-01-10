#pragma once

#include "fastfetch.h"

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
} FFBatteryResult;

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results);
