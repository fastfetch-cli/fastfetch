#pragma once

#include "fastfetch.h"

#define FF_CHASSIS_MODULE_NAME "Chassis"

void ffPrintChassis(FFChassisOptions* options);
void ffInitChassisOptions(FFChassisOptions* options);
void ffDestroyChassisOptions(FFChassisOptions* options);
