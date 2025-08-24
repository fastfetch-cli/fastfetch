#pragma once

#include "option.h"

#define FF_INITSYSTEM_MODULE_NAME "InitSystem"

bool ffPrintInitSystem(FFInitSystemOptions* options);
void ffInitInitSystemOptions(FFInitSystemOptions* options);
void ffDestroyInitSystemOptions(FFInitSystemOptions* options);

extern FFModuleBaseInfo ffInitSystemModuleInfo;
