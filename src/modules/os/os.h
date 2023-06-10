#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_OS_MODULE_NAME "OS"

void ffPrintOS(FFinstance* instance, FFOSOptions* options);
void ffInitOSOptions(FFOSOptions* options);
bool ffParseOSCommandOptions(FFOSOptions* options, const char* key, const char* value);
void ffDestroyOSOptions(FFOSOptions* options);
void ffParseOSJsonObject(FFinstance* instance, yyjson_val* module);
