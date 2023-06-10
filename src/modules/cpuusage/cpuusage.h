#pragma once

#include "fastfetch.h"

#define FF_CPUUSAGE_MODULE_NAME "CPUUsage"

void ffPrepareCPUUsage();

void ffPrintCPUUsage(FFinstance* instance, FFCPUUsageOptions* options);
void ffInitCPUUsageOptions(FFCPUUsageOptions* options);
bool ffParseCPUUsageCommandOptions(FFCPUUsageOptions* options, const char* key, const char* value);
void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options);
void ffParseCPUUsageJsonObject(FFinstance* instance, yyjson_val* module);
