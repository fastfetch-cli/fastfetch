#pragma once

#include "option.h"

#define FF_MEMORY_MODULE_NAME "Memory"

bool ffPrintMemory(FFMemoryOptions* options);
void ffInitMemoryOptions(FFMemoryOptions* options);
void ffDestroyMemoryOptions(FFMemoryOptions* options);

extern FFModuleBaseInfo ffMemoryModuleInfo;
