#pragma once

#include "option.h"

void ffPrepareCPUUsage();

bool ffPrintCPUUsage(FFCPUUsageOptions* options);
void ffInitCPUUsageOptions(FFCPUUsageOptions* options);
void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options);

extern FFModuleBaseInfo ffCPUUsageModuleInfo;
