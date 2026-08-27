#pragma once

#include "option.h"

bool ffPrintBreak(FFBreakOptions* options);
void ffInitBreakOptions(FFBreakOptions* options);
void ffDestroyBreakOptions(FFBreakOptions* options);

extern FFModuleBaseInfo ffBreakModuleInfo;
