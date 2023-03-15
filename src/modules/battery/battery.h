#pragma once

#include "fastfetch.h"

#define FF_BATTERY_MODULE_NAME "Battery"

void ffPrintBattery(FFinstance* instance, FFBatteryOptions* options);

void ffInitBatteryOptions(FFBatteryOptions* options);
bool ffParseBatteryCommandOptions(FFBatteryOptions* options, const char* key, const char* value);
void ffDestroyBatteryOptions(FFBatteryOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseBatteryJsonObject(FFinstance* instance, json_object* module);
#endif
