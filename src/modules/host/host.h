#pragma once

#include "fastfetch.h"

#define FF_HOST_MODULE_NAME "Host"

void ffPrintHost(FFinstance* instance, FFHostOptions* options);
void ffInitHostOptions(FFHostOptions* options);
bool ffParseHostCommandOptions(FFHostOptions* options, const char* key, const char* value);
void ffDestroyHostOptions(FFHostOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseHostJsonObject(FFinstance* instance, json_object* module);
#endif
