#pragma once

#include "option.h"

#define FF_UPTIME_MODULE_NAME "Uptime"

bool ffPrintUptime(FFUptimeOptions* options);
void ffInitUptimeOptions(FFUptimeOptions* options);
void ffDestroyUptimeOptions(FFUptimeOptions* options);

extern FFModuleBaseInfo ffUptimeModuleInfo;
