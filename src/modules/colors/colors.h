#pragma once

#include "option.h"

#define FF_COLORS_MODULE_NAME "Colors"

void ffPrintColors(FFColorsOptions* options);
void ffInitColorsOptions(FFColorsOptions* options);
void ffDestroyColorsOptions(FFColorsOptions* options);

extern FFModuleBaseInfo ffColorsModuleInfo;
