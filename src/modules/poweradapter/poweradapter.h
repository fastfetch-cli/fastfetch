#pragma once

#include "fastfetch.h"

#define FF_POWERADAPTER_MODULE_NAME "PowerAdapter"

void ffPrintPowerAdapter(FFPowerAdapterOptions* options);
void ffInitPowerAdapterOptions(FFPowerAdapterOptions* options);
bool ffParsePowerAdapterCommandOptions(FFPowerAdapterOptions* options, const char* key, const char* value);
void ffDestroyPowerAdapterOptions(FFPowerAdapterOptions* options);
void ffParsePowerAdapterJsonObject(FFPowerAdapterOptions* options, yyjson_val* module);
void ffGeneratePowerAdapterJson(FFPowerAdapterOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintPowerAdapterHelpFormat(void);
