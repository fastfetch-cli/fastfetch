#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_BREAK_MODULE_NAME "Break"

void ffPrintBreak(FFinstance* instance);
void ffParseBreakJsonObject(FFinstance* instance, yyjson_val* module);
