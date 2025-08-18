#pragma once

#include "option.h"

#define FF_GPU_MODULE_NAME "GPU"

bool ffPrintGPU(FFGPUOptions* options);
void ffInitGPUOptions(FFGPUOptions* options);
void ffDestroyGPUOptions(FFGPUOptions* options);

extern FFModuleBaseInfo ffGPUModuleInfo;
