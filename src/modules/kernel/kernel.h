#pragma once

#include "option.h"

#define FF_KERNEL_MODULE_NAME "Kernel"

bool ffPrintKernel(FFKernelOptions* options);
void ffInitKernelOptions(FFKernelOptions* options);
void ffDestroyKernelOptions(FFKernelOptions* options);

extern FFModuleBaseInfo ffKernelModuleInfo;
