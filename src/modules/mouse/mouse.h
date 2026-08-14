#pragma once

#include "option.h"

bool ffPrintMouse(FFMouseOptions* options);
void ffInitMouseOptions(FFMouseOptions* options);
void ffDestroyMouseOptions(FFMouseOptions* options);

extern FFModuleBaseInfo ffMouseModuleInfo;
