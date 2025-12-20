#pragma once

#include "option.h"

#define FF_CHASSIS_MODULE_NAME "Chassis"

bool ffPrintChassis(FFChassisOptions* options);
void ffInitChassisOptions(FFChassisOptions* options);
void ffDestroyChassisOptions(FFChassisOptions* options);

extern FFModuleBaseInfo ffChassisModuleInfo;
