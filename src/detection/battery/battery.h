#pragma once

#include "fastfetch.h"
#include "modules/battery/option.h"

#define FF_BATTERY_TEMP_UNSET (-DBL_MAX)

typedef enum FFBatteryStatus {
    FF_BATTERY_STATUS_NONE = 0,
    FF_BATTERY_STATUS_UNKNOWN = 1 << 0,
    FF_BATTERY_STATUS_AC_CONNECTED = 1 << 1,
    FF_BATTERY_STATUS_USB_CONNECTED = 1 << 2,
    FF_BATTERY_STATUS_WIRELESS_CONNECTED = 1 << 3,
    FF_BATTERY_STATUS_CHARGING = 1 << 4,
    FF_BATTERY_STATUS_DISCHARGING = 1 << 5,
    FF_BATTERY_STATUS_CRITICAL = 1 << 6,
} FFBatteryStatus;

typedef struct FFBatteryResult {
    FFstrbuf manufacturer;
    FFstrbuf manufactureDate;
    FFstrbuf modelName;
    FFstrbuf technology;
    FFstrbuf serial;
    FFBatteryStatus status;
    double capacity;
    double temperature;
    uint32_t cycleCount;
    int32_t timeRemaining; // in seconds, -1 if unknown
} FFBatteryResult;

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results);
