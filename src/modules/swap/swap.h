#pragma once

#include "option.h"

bool ffPrintSwap(FFSwapOptions* options);
void ffInitSwapOptions(FFSwapOptions* options);
void ffDestroySwapOptions(FFSwapOptions* options);

extern FFModuleBaseInfo ffSwapModuleInfo;
