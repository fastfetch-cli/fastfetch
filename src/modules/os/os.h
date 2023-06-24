#pragma once

#include "fastfetch.h"

#define FF_OS_MODULE_NAME "OS"

void ffPrintOS(FFOSOptions* options);
void ffInitOSOptions(FFOSOptions* options);
bool ffParseOSCommandOptions(FFOSOptions* options, const char* key, const char* value);
void ffDestroyOSOptions(FFOSOptions* options);
void ffParseOSJsonObject(yyjson_val* module);
