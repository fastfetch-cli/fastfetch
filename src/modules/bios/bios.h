#pragma once

#include "fastfetch.h"

#define FF_BIOS_MODULE_NAME "Bios"

void ffPrintBios(FFinstance* instance, FFBiosOptions* options);
void ffInitBiosOptions(FFBiosOptions* options);
bool ffParseBiosCommandOptions(FFBiosOptions* options, const char* key, const char* value);
void ffDestroyBiosOptions(FFBiosOptions* options);
void ffParseBiosJsonObject(FFinstance* instance, yyjson_val* module);
