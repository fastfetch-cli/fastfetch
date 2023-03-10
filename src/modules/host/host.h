#pragma once

#include "fastfetch.h"

void ffPrintHost(FFinstance* instance, FFHostOptions* options);
void ffInitHostOptions(FFHostOptions* options);
bool ffParseHostCommandOptions(FFHostOptions* options, const char* key, const char* value);
void ffDestroyHostOptions(FFHostOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/config.h"
bool ffParseHostJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module);
#endif
