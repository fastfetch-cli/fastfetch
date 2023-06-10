#pragma once

#include "fastfetch.h"

#define FF_PROCESSES_MODULE_NAME "Processes"

void ffPrintProcesses(FFinstance* instance, FFProcessesOptions* options);
void ffInitProcessesOptions(FFProcessesOptions* options);
bool ffParseProcessesCommandOptions(FFProcessesOptions* options, const char* key, const char* value);
void ffDestroyProcessesOptions(FFProcessesOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseProcessesJsonObject(FFinstance* instance, json_object* module);
#endif
