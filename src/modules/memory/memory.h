#pragma once

#include "fastfetch.h"

#define FF_MEMORY_MODULE_NAME "Memory"

void ffPrintMemory(FFMemoryOptions* options);
void ffInitMemoryOptions(FFMemoryOptions* options);
bool ffParseMemoryCommandOptions(FFMemoryOptions* options, const char* key, const char* value);
void ffDestroyMemoryOptions(FFMemoryOptions* options);
void ffParseMemoryJsonObject(yyjson_val* module);
