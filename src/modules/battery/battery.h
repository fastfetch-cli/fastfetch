#pragma once

#include "fastfetch.h"

#define FF_BATTERY_MODULE_NAME "Battery"

void ffPrintBattery(FFinstance* instance, FFBatteryOptions* options);

void ffInitBatteryOptions(FFBatteryOptions* options);
bool ffParseBatteryCommandOptions(FFBatteryOptions* options, const char* key, const char* value);
void ffDestroyBatteryOptions(FFBatteryOptions* options);
void ffParseBatteryJsonObject(FFinstance* instance, yyjson_val* module);
