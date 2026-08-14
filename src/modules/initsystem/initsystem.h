#pragma once

#include "option.h"

bool ffPrintInitSystem(FFInitSystemOptions* options);
void ffInitInitSystemOptions(FFInitSystemOptions* options);
void ffDestroyInitSystemOptions(FFInitSystemOptions* options);

extern FFModuleBaseInfo ffInitSystemModuleInfo;
