#pragma once

#include "fastfetch.h"

#define FF_UPTIME_MODULE_NAME "Uptime"

void ffPrintUptime(FFUptimeOptions* options);
void ffInitUptimeOptions(FFUptimeOptions* options);
void ffDestroyUptimeOptions(FFUptimeOptions* options);
