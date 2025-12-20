#pragma once

#include "option.h"

#define FF_CPUUSAGE_MODULE_NAME "CPUUsage"

void ffPrepareCPUUsage();

bool ffPrintCPUUsage(FFCPUUsageOptions* options);
void ffInitCPUUsageOptions(FFCPUUsageOptions* options);
void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options);

extern FFModuleBaseInfo ffCPUUsageModuleInfo;
