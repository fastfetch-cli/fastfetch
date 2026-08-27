#pragma once

#include "option.h"

bool ffPrintBattery(FFBatteryOptions* options);
void ffInitBatteryOptions(FFBatteryOptions* options);
void ffDestroyBatteryOptions(FFBatteryOptions* options);

extern FFModuleBaseInfo ffBatteryModuleInfo;
