#pragma once

#include "option.h"

bool ffPrintKernel(FFKernelOptions* options);
void ffInitKernelOptions(FFKernelOptions* options);
void ffDestroyKernelOptions(FFKernelOptions* options);

extern FFModuleBaseInfo ffKernelModuleInfo;
