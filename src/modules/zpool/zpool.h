#pragma once

#include "option.h"

bool ffPrintZpool(FFZpoolOptions* options);
void ffInitZpoolOptions(FFZpoolOptions* options);
void ffDestroyZpoolOptions(FFZpoolOptions* options);

extern FFModuleBaseInfo ffZpoolModuleInfo;
