#pragma once

#include "fastfetch.h"

#define FF_CPU_MODULE_NAME "CPU"

void ffPrintCPU(FFinstance* instance, FFCPUOptions* options);
void ffInitCPUOptions(FFCPUOptions* options);
bool ffParseCPUCommandOptions(FFCPUOptions* options, const char* key, const char* value);
void ffDestroyCPUOptions(FFCPUOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseCPUJsonObject(FFinstance* instance, json_object* module);
#endif
