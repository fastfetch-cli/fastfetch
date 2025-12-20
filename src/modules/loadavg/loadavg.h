#pragma once

#include "option.h"

#define FF_LOADAVG_MODULE_NAME "Loadavg"

bool ffPrintLoadavg(FFLoadavgOptions* options);
void ffInitLoadavgOptions(FFLoadavgOptions* options);
void ffDestroyLoadavgOptions(FFLoadavgOptions* options);

extern FFModuleBaseInfo ffLoadavgModuleInfo;
