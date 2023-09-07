#pragma once

#include "fastfetch.h"

#define FF_OS_MODULE_NAME "OS"

void ffPrintOS(FFOSOptions* options);
void ffInitOSOptions(FFOSOptions* options);
bool ffParseOSCommandOptions(FFOSOptions* options, const char* key, const char* value);
void ffDestroyOSOptions(FFOSOptions* options);
void ffParseOSJsonObject(FFOSOptions* options, yyjson_val* module);
void ffGenerateOSJson(FFOSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
