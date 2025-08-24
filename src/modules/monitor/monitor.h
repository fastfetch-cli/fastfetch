#pragma once

#include "option.h"

#define FF_MONITOR_MODULE_NAME "Monitor"

bool ffPrintMonitor(FFMonitorOptions* options);
void ffInitMonitorOptions(FFMonitorOptions* options);
void ffDestroyMonitorOptions(FFMonitorOptions* options);

extern FFModuleBaseInfo ffMonitorModuleInfo;
