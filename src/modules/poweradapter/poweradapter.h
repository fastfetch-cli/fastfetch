#pragma once

#include "option.h"

bool ffPrintPowerAdapter(FFPowerAdapterOptions* options);
void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options);
void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options);

extern FFModuleBaseInfo ffPowerAdapterModuleInfo;
