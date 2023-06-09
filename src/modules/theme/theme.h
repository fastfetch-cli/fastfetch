#pragma once

#include "fastfetch.h"

#define FF_THEME_MODULE_NAME "Theme"

void ffPrintTheme(FFinstance* instance, FFThemeOptions* options);
void ffInitThemeOptions(FFThemeOptions* options);
bool ffParseThemeCommandOptions(FFThemeOptions* options, const char* key, const char* value);
void ffDestroyThemeOptions(FFThemeOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseThemeJsonObject(FFinstance* instance, json_object* module);
#endif
