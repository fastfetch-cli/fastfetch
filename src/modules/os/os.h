#pragma once

#include "option.h"

#define FF_OS_MODULE_NAME "OS"

bool ffPrintOS(FFOSOptions* options);
void ffInitOSOptions(FFOSOptions* options);
void ffDestroyOSOptions(FFOSOptions* options);

extern FFModuleBaseInfo ffOSModuleInfo;
