#pragma once

#include "fastfetch.h"

#define FF_CPUUSAGE_MODULE_NAME "CPUUsage"

void ffPrepareCPUUsage();

void ffPrintCPUUsage(FFCPUUsageOptions* options);
void ffInitCPUUsageOptions(FFCPUUsageOptions* options);
void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options);
