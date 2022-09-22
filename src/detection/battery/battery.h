#pragma once

#ifndef FF_INCLUDED_detection_battery_battery
#define FF_INCLUDED_detection_battery_battery

#include "fastfetch.h"

typedef struct BatteryResult
{
    FFstrbuf manufacturer;
    FFstrbuf modelName;
    FFstrbuf technology;
    FFstrbuf capacity;
    FFstrbuf status;
    FFstrbuf adapterName;
    int adapterWatts;
} BatteryResult;

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results);

#endif
