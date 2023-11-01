#pragma once

#include "fastfetch.h"

#define FF_MONITOR_MODULE_NAME "Monitor"

void ffPrintMonitor(FFMonitorOptions* options);
void ffInitMonitorOptions(FFMonitorOptions* options);
void ffDestroyMonitorOptions(FFMonitorOptions* options);
