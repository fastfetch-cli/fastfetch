#pragma once

#include "option.h"

#define FF_SWAP_MODULE_NAME "Swap"

bool ffPrintSwap(FFSwapOptions* options);
void ffInitSwapOptions(FFSwapOptions* options);
void ffDestroySwapOptions(FFSwapOptions* options);

extern FFModuleBaseInfo ffSwapModuleInfo;
