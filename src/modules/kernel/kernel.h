#pragma once

#include "fastfetch.h"

#define FF_KERNEL_MODULE_NAME "Kernel"

void ffPrintKernel(FFKernelOptions* options);
void ffInitKernelOptions(FFKernelOptions* options);
void ffDestroyKernelOptions(FFKernelOptions* options);
