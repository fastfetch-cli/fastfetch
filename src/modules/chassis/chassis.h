#pragma once

#include "option.h"

bool ffPrintChassis(FFChassisOptions* options);
void ffInitChassisOptions(FFChassisOptions* options);
void ffDestroyChassisOptions(FFChassisOptions* options);

extern FFModuleBaseInfo ffChassisModuleInfo;
