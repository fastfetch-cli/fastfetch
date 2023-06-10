#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_CPU_MODULE_NAME "CPU"

void ffPrintCPU(FFinstance* instance, FFCPUOptions* options);
void ffInitCPUOptions(FFCPUOptions* options);
bool ffParseCPUCommandOptions(FFCPUOptions* options, const char* key, const char* value);
void ffDestroyCPUOptions(FFCPUOptions* options);
void ffParseCPUJsonObject(FFinstance* instance, yyjson_val* module);
