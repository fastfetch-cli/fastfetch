#pragma once

#include "option.h"

#define FF_MOUSE_MODULE_NAME "Mouse"

bool ffPrintMouse(FFMouseOptions* options);
void ffInitMouseOptions(FFMouseOptions* options);
void ffDestroyMouseOptions(FFMouseOptions* options);

extern FFModuleBaseInfo ffMouseModuleInfo;
