#pragma once

#include "fastfetch.h"

#define FF_POWERADAPTER_MODULE_NAME "PowerAdapter"

void ffPrintPowerAdapter(FFinstance* instance, FFPowerAdapterOptions* options);
void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options);
bool ffParsePowerAdapterCommandOptions(FFPowerAdapterOptions* options, const char* key, const char* value);
void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options);
void ffParsePowerAdapterJsonObject(FFinstance* instance, yyjson_val* module);
