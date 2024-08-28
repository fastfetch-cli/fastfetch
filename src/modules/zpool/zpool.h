#pragma once

#include "fastfetch.h"

#define FF_ZPOOL_MODULE_NAME "Zpool"

void ffPrintZpool(FFZpoolOptions* options);
void ffInitZpoolOptions(FFZpoolOptions* options);
void ffDestroyZpoolOptions(FFZpoolOptions* options);
