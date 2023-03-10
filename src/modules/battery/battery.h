#pragma once

#include "fastfetch.h"

void ffPrintBattery(FFinstance* instance, FFBatteryOptions* options);

void ffInitBatteryOptions(FFBatteryOptions* options);
bool ffParseBatteryCommandOptions(FFBatteryOptions* options, const char* key, const char* value);
void ffDestroyBatteryOptions(FFBatteryOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/config.h"
bool ffParseBatteryJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module);
#endif
