#pragma once

#include "fastfetch.h"

void ffPrintBios(FFinstance* instance, FFBiosOptions* options);
void ffInitBiosOptions(FFBiosOptions* options);
bool ffParseBiosCommandOptions(FFBiosOptions* options, const char* key, const char* value);
void ffDestroyBiosOptions(FFBiosOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
bool ffParseBiosJsonObject(FFinstance* instance, const char* type, json_object* module);
#endif
