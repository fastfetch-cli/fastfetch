#pragma once

#include "fastfetch.h"

#define FF_COLORS_MODULE_NAME "Colors"

void ffPrintColors(FFinstance* instance, FFColorsOptions* options);
void ffInitColorsOptions(FFColorsOptions* options);
void ffDestroyColorsOptions(FFColorsOptions* options);
void ffParseColorsJsonObject(FFinstance* instance, yyjson_val* module);
