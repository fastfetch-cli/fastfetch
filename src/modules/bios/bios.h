#pragma once

#include "fastfetch.h"

#define FF_BIOS_MODULE_NAME "Bios"

void ffPrintBios(FFBiosOptions* options);
void ffInitBiosOptions(FFBiosOptions* options);
bool ffParseBiosCommandOptions(FFBiosOptions* options, const char* key, const char* value);
void ffDestroyBiosOptions(FFBiosOptions* options);
void ffParseBiosJsonObject(FFBiosOptions* options, yyjson_val* module);
void ffGenerateBiosJson(FFBiosOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
