#pragma once

#include "fastfetch.h"

#define FF_MEMORY_MODULE_NAME "Memory"

void ffPrintMemory(FFMemoryOptions* options);
void ffInitMemoryOptions(FFMemoryOptions* options);
void ffDestroyMemoryOptions(FFMemoryOptions* options);
