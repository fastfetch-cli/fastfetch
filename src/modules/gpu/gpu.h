#pragma once

#include "fastfetch.h"

#define FF_GPU_MODULE_NAME "GPU"

void ffPrintGPU(FFGPUOptions* options);
void ffInitGPUOptions(FFGPUOptions* options);
bool ffParseGPUCommandOptions(FFGPUOptions* options, const char* key, const char* value);
void ffDestroyGPUOptions(FFGPUOptions* options);
void ffParseGPUJsonObject(FFGPUOptions* options, yyjson_val* module);
void ffGenerateGPUJson(FFGPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintGPUHelpFormat(void);
