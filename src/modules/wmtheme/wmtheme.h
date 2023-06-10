#pragma once

#include "fastfetch.h"

#define FF_WMTHEME_MODULE_NAME "WMTheme"

void ffPrintWMTheme(FFinstance* instance, FFWMThemeOptions* options);
void ffInitWMThemeOptions(FFWMThemeOptions* options);
bool ffParseWMThemeCommandOptions(FFWMThemeOptions* options, const char* key, const char* value);
void ffDestroyWMThemeOptions(FFWMThemeOptions* options);
void ffParseWMThemeJsonObject(FFinstance* instance, yyjson_val* module);
