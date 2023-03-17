#pragma once

#define FF_CPUUSAGE_MODULE_NAME "CPUUsage"

void ffPrepareCPUUsage();

void ffPrintCPUUsage(FFinstance* instance, FFCPUUsageOptions* options);
void ffInitCPUUsageOptions(FFCPUUsageOptions* options);
bool ffParseCPUUsageCommandOptions(FFCPUUsageOptions* options, const char* key, const char* value);
void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseCPUUsageJsonObject(FFinstance* instance, json_object* module);
#endif
