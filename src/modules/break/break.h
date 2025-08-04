#pragma once

#include "option.h"

#define FF_BREAK_MODULE_NAME "Break"

void ffPrintBreak(FFBreakOptions* options);
void ffInitBreakOptions(FFBreakOptions* options);
void ffDestroyBreakOptions(FFBreakOptions* options);

extern FFModuleBaseInfo ffBreakModuleInfo;
