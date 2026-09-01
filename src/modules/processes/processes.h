#pragma once

#include "option.h"

bool ffPrintProcesses(FFProcessesOptions* options);
void ffInitProcessesOptions(FFProcessesOptions* options);
void ffDestroyProcessesOptions(FFProcessesOptions* options);

extern FFModuleBaseInfo ffProcessesModuleInfo;
