#pragma once

#include "fastfetch.h"

#define FF_BIOS_MODULE_NAME "Bios"

void ffPrintBios(FFinstance* instance, FFBiosOptions* options);
void ffInitBiosOptions(FFBiosOptions* options);
bool ffParseBiosCommandOptions(FFBiosOptions* options, const char* key, const char* value);
void ffDestroyBiosOptions(FFBiosOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseBiosJsonObject(FFinstance* instance, json_object* module);
#endif
