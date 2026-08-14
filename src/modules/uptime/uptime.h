#pragma once

#include "option.h"

bool ffPrintUptime(FFUptimeOptions* options);
void ffInitUptimeOptions(FFUptimeOptions* options);
void ffDestroyUptimeOptions(FFUptimeOptions* options);

extern FFModuleBaseInfo ffUptimeModuleInfo;
