#pragma once

#include "fastfetch.h"

#define FF_CPU_MODULE_NAME "CPU"

void ffPrintCPU(FFCPUOptions* options);
void ffInitCPUOptions(FFCPUOptions* options);
void ffDestroyCPUOptions(FFCPUOptions* options);
