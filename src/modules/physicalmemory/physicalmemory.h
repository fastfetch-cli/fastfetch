#pragma once

#include "option.h"

bool ffPrintPhysicalMemory(FFPhysicalMemoryOptions* options);
void ffInitPhysicalMemoryOptions(FFPhysicalMemoryOptions* options);
void ffDestroyPhysicalMemoryOptions(FFPhysicalMemoryOptions* options);

extern FFModuleBaseInfo ffPhysicalMemoryModuleInfo;
