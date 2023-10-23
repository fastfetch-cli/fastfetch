#pragma once

#include "fastfetch.h"

#define FF_BATTERY_MODULE_NAME "Battery"

void ffPrintBattery(FFBatteryOptions* options);
void ffInitBatteryOptions(FFBatteryOptions* options);
void ffDestroyBatteryOptions(FFBatteryOptions* options);
