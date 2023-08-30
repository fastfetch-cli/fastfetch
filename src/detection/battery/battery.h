#pragma once

#ifndef FF_INCLUDED_detection_battery_battery
#define FF_INCLUDED_detection_battery_battery

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
} FFBatteryResult;

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results);

#endif
