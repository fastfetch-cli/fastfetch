#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_PROCESSES_MODULE_NAME "Processes"

void ffPrintProcesses(FFinstance* instance, FFProcessesOptions* options);
void ffInitProcessesOptions(FFProcessesOptions* options);
bool ffParseProcessesCommandOptions(FFProcessesOptions* options, const char* key, const char* value);
void ffDestroyProcessesOptions(FFProcessesOptions* options);
void ffParseProcessesJsonObject(FFinstance* instance, yyjson_val* module);
