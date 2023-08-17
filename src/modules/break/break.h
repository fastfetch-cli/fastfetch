#pragma once

#include "fastfetch.h"

#define FF_BREAK_MODULE_NAME "Break"

void ffPrintBreak(FFBreakOptions* options);
void ffInitBreakOptions(FFBreakOptions* options);
bool ffParseBreakCommandOptions(FFBreakOptions* options, const char* key, const char* value);
void ffDestroyBreakOptions(FFBreakOptions* options);
void ffParseBreakJsonObject(FFBreakOptions* options, yyjson_val* module);
