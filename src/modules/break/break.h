#pragma once

#include "fastfetch.h"

#define FF_BREAK_MODULE_NAME "Break"

void ffPrintBreak();
void ffParseBreakJsonObject(yyjson_val* module);
