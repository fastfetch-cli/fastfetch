#pragma once

#include "fastfetch.h"

#define FF_MONITOR_MODULE_NAME "Monitor"

void ffPrintMonitor(FFMonitorOptions* options);
void ffInitMonitorOptions(FFMonitorOptions* options);
bool ffParseMonitorCommandOptions(FFMonitorOptions* options, const char* key, const char* value);
void ffDestroyMonitorOptions(FFMonitorOptions* options);
void ffParseMonitorJsonObject(FFMonitorOptions* options, yyjson_val* module);
void ffGenerateMonitorJson(FFMonitorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
