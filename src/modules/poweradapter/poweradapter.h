#pragma once

#include "fastfetch.h"

#define FF_POWERADAPTER_MODULE_NAME "PowerAdapter"

void ffPrintPowerAdapter(FFinstance* instance, FFPowerAdapterOptions* options);
void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options);
bool ffParsePowerAdapterCommandOptions(FFPowerAdapterOptions* options, const char* key, const char* value);
void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParsePowerAdapterJsonObject(FFinstance* instance, json_object* module);
#endif
