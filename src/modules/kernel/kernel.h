#pragma once

#include "fastfetch.h"
#include "modules/kernel/option.h"

#define FF_KERNEL_MODULE_NAME "Kernel"

void ffPrintKernel(FFinstance* instance, FFKernelOptions* options);
void ffInitKernelOptions(FFKernelOptions* options);
bool ffParseKernelCommandOptions(FFKernelOptions* options, const char* key, const char* value);
void ffDestroyKernelOptions(FFKernelOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseKernelJsonObject(FFinstance* instance, json_object* module);
#endif
