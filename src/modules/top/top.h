#pragma once

#include "option.h"

void ffPrepareTopProcesses(FFTopTypes showTypes);

bool ffPrintTop(FFTopOptions* options);
void ffInitTopOptions(FFTopOptions* options);
void ffDestroyTopOptions(FFTopOptions* options);

extern FFModuleBaseInfo ffTopModuleInfo;
