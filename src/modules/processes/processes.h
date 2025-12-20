#pragma once

#include "option.h"

#define FF_PROCESSES_MODULE_NAME "Processes"

bool ffPrintProcesses(FFProcessesOptions* options);
void ffInitProcessesOptions(FFProcessesOptions* options);
void ffDestroyProcessesOptions(FFProcessesOptions* options);

extern FFModuleBaseInfo ffProcessesModuleInfo;
