#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_GPU_MODULE_NAME "GPU"

void ffPrintGPU(FFinstance* instance, FFGPUOptions* options);
void ffInitGPUOptions(FFGPUOptions* options);
bool ffParseGPUCommandOptions(FFGPUOptions* options, const char* key, const char* value);
void ffDestroyGPUOptions(FFGPUOptions* options);
void ffParseGPUJsonObject(FFinstance* instance, yyjson_val* module);
