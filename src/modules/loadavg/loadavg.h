#pragma once

#include "fastfetch.h"

#define FF_LOADAVG_MODULE_NAME "Loadavg"

void ffPrintLoadavg(FFLoadavgOptions* options);
void ffInitLoadavgOptions(FFLoadavgOptions* options);
void ffDestroyLoadavgOptions(FFLoadavgOptions* options);
