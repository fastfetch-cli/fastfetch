#pragma once

#include "option.h"

#define FF_ZPOOL_MODULE_NAME "Zpool"

bool ffPrintZpool(FFZpoolOptions* options);
void ffInitZpoolOptions(FFZpoolOptions* options);
void ffDestroyZpoolOptions(FFZpoolOptions* options);

extern FFModuleBaseInfo ffZpoolModuleInfo;
