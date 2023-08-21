#pragma once

#include "fastfetch.h"

#define FF_PROCESSES_MODULE_NAME "Processes"

void ffPrintProcesses(FFProcessesOptions* options);
void ffInitProcessesOptions(FFProcessesOptions* options);
bool ffParseProcessesCommandOptions(FFProcessesOptions* options, const char* key, const char* value);
void ffDestroyProcessesOptions(FFProcessesOptions* options);
void ffParseProcessesJsonObject(FFProcessesOptions* options, yyjson_val* module);
