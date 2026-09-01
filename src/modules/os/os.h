#pragma once

#include "option.h"

bool ffPrintOS(FFOSOptions* options);
void ffInitOSOptions(FFOSOptions* options);
void ffDestroyOSOptions(FFOSOptions* options);

extern FFModuleBaseInfo ffOSModuleInfo;
