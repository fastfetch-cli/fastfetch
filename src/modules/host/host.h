#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_HOST_MODULE_NAME "Host"

void ffPrintHost(FFinstance* instance, FFHostOptions* options);
void ffInitHostOptions(FFHostOptions* options);
bool ffParseHostCommandOptions(FFHostOptions* options, const char* key, const char* value);
void ffDestroyHostOptions(FFHostOptions* options);
void ffParseHostJsonObject(FFinstance* instance, yyjson_val* module);
