#pragma once

#include "option.h"

bool ffPrintCPU(FFCPUOptions* options);
void ffInitCPUOptions(FFCPUOptions* options);
void ffDestroyCPUOptions(FFCPUOptions* options);

extern FFModuleBaseInfo ffCPUModuleInfo;
