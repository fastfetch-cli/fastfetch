#pragma once

#include "option.h"

bool ffPrintGPU(FFGPUOptions* options);
void ffInitGPUOptions(FFGPUOptions* options);
void ffDestroyGPUOptions(FFGPUOptions* options);

extern FFModuleBaseInfo ffGPUModuleInfo;
