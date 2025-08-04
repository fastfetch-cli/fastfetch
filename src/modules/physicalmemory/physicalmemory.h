#pragma once

#include "option.h"

#define FF_PHYSICALMEMORY_MODULE_NAME "PhysicalMemory"

void ffPrintPhysicalMemory(FFPhysicalMemoryOptions* options);
void ffInitPhysicalMemoryOptions(FFPhysicalMemoryOptions* options);
void ffDestroyPhysicalMemoryOptions(FFPhysicalMemoryOptions* options);

extern FFModuleBaseInfo ffPhysicalMemoryModuleInfo;
