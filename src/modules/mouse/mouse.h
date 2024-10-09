#pragma once

#include "fastfetch.h"

#define FF_MOUSE_MODULE_NAME "Mouse"

void ffPrintMouse(FFMouseOptions* options);
void ffInitMouseOptions(FFMouseOptions* options);
void ffDestroyMouseOptions(FFMouseOptions* options);
