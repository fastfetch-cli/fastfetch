#pragma once

#include "option.h"

#define FF_HOST_MODULE_NAME "Host"

bool ffPrintHost(FFHostOptions* options);
void ffInitHostOptions(FFHostOptions* options);
void ffDestroyHostOptions(FFHostOptions* options);

extern FFModuleBaseInfo ffHostModuleInfo;
