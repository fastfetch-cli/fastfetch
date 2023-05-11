#pragma once

#include "fastfetch.h"

#define FF_MEMORY_MODULE_NAME "Memory"

void ffPrintMemory(FFinstance* instance, FFMemoryOptions* options);
void ffInitMemoryOptions(FFMemoryOptions* options);
bool ffParseMemoryCommandOptions(FFMemoryOptions* options, const char* key, const char* value);
void ffDestroyMemoryOptions(FFMemoryOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseMemoryJsonObject(FFinstance* instance, json_object* module);
#endif
