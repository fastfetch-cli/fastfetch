#pragma once

#include "option.h"

void ffPrepareTopProcesses(void);

bool ffPrintTop(FFTopOptions* options);
void ffInitTopOptions(FFTopOptions* options);
void ffDestroyTopOptions(FFTopOptions* options);

extern FFModuleBaseInfo ffTopModuleInfo;
