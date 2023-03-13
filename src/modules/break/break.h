#pragma once

#include "fastfetch.h"

void ffPrintBreak(FFinstance* instance);

#ifdef FF_HAVE_JSONC
#include "common/config.h"
bool ffParseBreakJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module);
#endif
