#pragma once

#include "fastfetch.h"

#define FF_GPU_MODULE_NAME "GPU"

void ffPrintGPU(FFGPUOptions* options);
void ffInitGPUOptions(FFGPUOptions* options);
void ffDestroyGPUOptions(FFGPUOptions* options);
