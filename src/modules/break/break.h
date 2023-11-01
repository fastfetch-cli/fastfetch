#pragma once

#include "fastfetch.h"

#define FF_BREAK_MODULE_NAME "Break"

void ffPrintBreak(FFBreakOptions* options);
void ffInitBreakOptions(FFBreakOptions* options);
void ffDestroyBreakOptions(FFBreakOptions* options);
