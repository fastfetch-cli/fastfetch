#pragma once

#include "fastfetch.h"

#define FF_POWERADAPTER_MODULE_NAME "PowerAdapter"

void ffPrintPowerAdapter(FFPowerAdapterOptions* options);
void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options);
void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options);
