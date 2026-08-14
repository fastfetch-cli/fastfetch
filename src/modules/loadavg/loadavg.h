#pragma once

#include "option.h"

bool ffPrintLoadavg(FFLoadavgOptions* options);
void ffInitLoadavgOptions(FFLoadavgOptions* options);
void ffDestroyLoadavgOptions(FFLoadavgOptions* options);

extern FFModuleBaseInfo ffLoadavgModuleInfo;
