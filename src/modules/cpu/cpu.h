#pragma once

#include "option.h"

#define FF_CPU_MODULE_NAME "CPU"

bool ffPrintCPU(FFCPUOptions* options);
void ffInitCPUOptions(FFCPUOptions* options);
void ffDestroyCPUOptions(FFCPUOptions* options);

extern FFModuleBaseInfo ffCPUModuleInfo;
