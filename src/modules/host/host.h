#pragma once

#include "option.h"

bool ffPrintHost(FFHostOptions* options);
void ffInitHostOptions(FFHostOptions* options);
void ffDestroyHostOptions(FFHostOptions* options);

extern FFModuleBaseInfo ffHostModuleInfo;
