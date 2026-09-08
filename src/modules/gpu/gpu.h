#pragma once

#include "option.h"
#include "detection/gpu/gpu.h"

bool ffPrintGPU(FFGPUOptions* options);
void ffGPUAppendName(const FFGPUResult* gpu, FFstrbuf* output);
void ffInitGPUOptions(FFGPUOptions* options);
void ffDestroyGPUOptions(FFGPUOptions* options);

extern FFModuleBaseInfo ffGPUModuleInfo;
