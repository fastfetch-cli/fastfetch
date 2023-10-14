#pragma once

#include "fastfetch.h"

#define FF_CPU_MODULE_NAME "CPU"

void ffPrintCPU(FFCPUOptions* options);
void ffInitCPUOptions(FFCPUOptions* options);
bool ffParseCPUCommandOptions(FFCPUOptions* options, const char* key, const char* value);
void ffDestroyCPUOptions(FFCPUOptions* options);
void ffParseCPUJsonObject(FFCPUOptions* options, yyjson_val* module);
void ffGenerateCPUJson(FFCPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintCPUHelpFormat(void);
