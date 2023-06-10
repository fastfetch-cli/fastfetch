#pragma once

#include "fastfetch.h"

#define FF_BREAK_MODULE_NAME "Break"

void ffPrintBreak(FFinstance* instance);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseBreakJsonObject(FFinstance* instance, json_object* module);
#endif
