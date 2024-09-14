#pragma once

#include "fastfetch.h"

#define FF_ABI_MODULE_NAME "ABI"

void ffPrintABI(FFABIOptions* options);
void ffInitABIOptions(FFABIOptions* options);
void ffDestroyABIOptions(FFABIOptions* options);
