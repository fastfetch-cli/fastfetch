#pragma once

#include "fastfetch.h"
#include "modules/battery/option.h"

void ffPrintBattery(FFinstance* instance, FFBatteryOptions* options);

void ffInitBatteryOptions(FFBatteryOptions* options);
bool ffParseBatteryCommandOptions(FFBatteryOptions* options, const char* key, const char* value);
void ffDestroyBatteryOptions(FFBatteryOptions* options);
