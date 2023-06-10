#pragma once

#include "fastfetch.h"

#define FF_KERNEL_MODULE_NAME "Kernel"

void ffPrintKernel(FFinstance* instance, FFKernelOptions* options);
void ffInitKernelOptions(FFKernelOptions* options);
bool ffParseKernelCommandOptions(FFKernelOptions* options, const char* key, const char* value);
void ffDestroyKernelOptions(FFKernelOptions* options);
void ffParseKernelJsonObject(FFinstance* instance, yyjson_val* module);
