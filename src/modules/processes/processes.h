#pragma once

#include "fastfetch.h"

#define FF_PROCESSES_MODULE_NAME "Processes"

void ffPrintProcesses(FFProcessesOptions* options);
void ffInitProcessesOptions(FFProcessesOptions* options);
void ffDestroyProcessesOptions(FFProcessesOptions* options);
