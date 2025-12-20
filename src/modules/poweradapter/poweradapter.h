#pragma once

#include "option.h"

#define FF_POWERADAPTER_MODULE_NAME "PowerAdapter"

bool ffPrintPowerAdapter(FFPowerAdapterOptions* options);
void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options);
void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options);

extern FFModuleBaseInfo ffPowerAdapterModuleInfo;
