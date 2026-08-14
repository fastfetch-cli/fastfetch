#pragma once

#include "option.h"

bool ffPrintMemory(FFMemoryOptions* options);
void ffInitMemoryOptions(FFMemoryOptions* options);
void ffDestroyMemoryOptions(FFMemoryOptions* options);

extern FFModuleBaseInfo ffMemoryModuleInfo;
