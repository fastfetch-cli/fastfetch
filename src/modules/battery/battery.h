#pragma once

#include "fastfetch.h"

#define FF_BATTERY_MODULE_NAME "Battery"

void ffPrintBattery(FFBatteryOptions* options);

void ffInitBatteryOptions(FFBatteryOptions* options);
bool ffParseBatteryCommandOptions(FFBatteryOptions* options, const char* key, const char* value);
void ffDestroyBatteryOptions(FFBatteryOptions* options);
void ffParseBatteryJsonObject(FFBatteryOptions* options, yyjson_val* module);
