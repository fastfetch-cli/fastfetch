#pragma once

#include "option.h"

bool ffPrintMonitor(FFMonitorOptions* options);
void ffInitMonitorOptions(FFMonitorOptions* options);
void ffDestroyMonitorOptions(FFMonitorOptions* options);

extern FFModuleBaseInfo ffMonitorModuleInfo;
