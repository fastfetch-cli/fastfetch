#pragma once

#include "option.h"

bool ffPrintColors(FFColorsOptions* options);
void ffInitColorsOptions(FFColorsOptions* options);
void ffDestroyColorsOptions(FFColorsOptions* options);

extern FFModuleBaseInfo ffColorsModuleInfo;
