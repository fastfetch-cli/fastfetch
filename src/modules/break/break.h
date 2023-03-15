#pragma once

#include "fastfetch.h"

void ffPrintBreak(FFinstance* instance);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
bool ffParseBreakJsonObject(FFinstance* instance, const char* type, json_object* module);
#endif
