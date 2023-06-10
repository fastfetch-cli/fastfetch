#pragma once

#include "fastfetch.h"
#include "modules/wmtheme/option.h"

#define FF_WMTHEME_MODULE_NAME "WMTheme"

void ffPrintWMTheme(FFinstance* instance, FFWMThemeOptions* options);
void ffInitWMThemeOptions(FFWMThemeOptions* options);
bool ffParseWMThemeCommandOptions(FFWMThemeOptions* options, const char* key, const char* value);
void ffDestroyWMThemeOptions(FFWMThemeOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseWMThemeJsonObject(FFinstance* instance, json_object* module);
#endif
