#pragma once

#include "fastfetch.h"

#define FF_WMTHEME_MODULE_NAME "WMTheme"

void ffPrintWMTheme(FFWMThemeOptions* options);
void ffInitWMThemeOptions(FFWMThemeOptions* options);
bool ffParseWMThemeCommandOptions(FFWMThemeOptions* options, const char* key, const char* value);
void ffDestroyWMThemeOptions(FFWMThemeOptions* options);
void ffParseWMThemeJsonObject(yyjson_val* module);
