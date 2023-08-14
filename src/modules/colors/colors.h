#pragma once

#include "fastfetch.h"

#define FF_COLORS_MODULE_NAME "Colors"

void ffPrintColors(FFColorsOptions* options);
void ffInitColorsOptions(FFColorsOptions* options);
void ffDestroyColorsOptions(FFColorsOptions* options);
bool ffParseColorsCommandOptions(FFColorsOptions* options, const char* key, const char* value);
void ffParseColorsJsonObject(yyjson_val* module);
