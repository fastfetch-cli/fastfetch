#pragma once

#include "fastfetch.h"

#define FF_COLORS_MODULE_NAME "Colors"

void ffPrintColors(FFinstance* instance);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseColorsJsonObject(FFinstance* instance, json_object* module);
#endif
