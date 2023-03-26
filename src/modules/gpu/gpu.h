#pragma once

#define FF_GPU_MODULE_NAME "GPU"

void ffPrintGPU(FFinstance* instance, FFGPUOptions* options);
void ffInitGPUOptions(FFGPUOptions* options);
bool ffParseGPUCommandOptions(FFGPUOptions* options, const char* key, const char* value);
void ffDestroyGPUOptions(FFGPUOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseGPUJsonObject(FFinstance* instance, json_object* module);
#endif
