#pragma once

#include "option.h"

#define FF_BATTERY_MODULE_NAME "Battery"

void ffPrintBattery(FFBatteryOptions* options);
void ffInitBatteryOptions(FFBatteryOptions* options);
void ffDestroyBatteryOptions(FFBatteryOptions* options);

extern FFModuleBaseInfo ffBatteryModuleInfo;
