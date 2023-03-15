#pragma once

#include "fastfetch.h"
#include "modules/kernel/option.h"

void ffPrintKernel(FFinstance* instance, FFKernelOptions* options);
void ffInitKernelOptions(FFKernelOptions* options);
bool ffParseKernelCommandOptions(FFKernelOptions* options, const char* key, const char* value);
void ffDestroyKernelOptions(FFKernelOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
bool ffParseKernelJsonObject(FFinstance* instance, const char* type, json_object* module);
#endif
