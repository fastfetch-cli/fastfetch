#pragma once

#include "fastfetch.h"

#define FF_WMTHEME_MODULE_NAME "WMTheme"

void ffPrintWMTheme(FFWMThemeOptions* options);
void ffInitWMThemeOptions(FFWMThemeOptions* options);
bool ffParseWMThemeCommandOptions(FFWMThemeOptions* options, const char* key, const char* value);
void ffDestroyWMThemeOptions(FFWMThemeOptions* options);
void ffParseWMThemeJsonObject(FFWMThemeOptions* options, yyjson_val* module);
void ffGenerateWMThemeJson(FFWMThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
