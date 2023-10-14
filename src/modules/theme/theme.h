#pragma once

#include "fastfetch.h"

#define FF_THEME_MODULE_NAME "Theme"

void ffPrintTheme(FFThemeOptions* options);
void ffInitThemeOptions(FFThemeOptions* options);
bool ffParseThemeCommandOptions(FFThemeOptions* options, const char* key, const char* value);
void ffDestroyThemeOptions(FFThemeOptions* options);
void ffParseThemeJsonObject(FFThemeOptions* options, yyjson_val* module);
void ffGenerateThemeJson(FFThemeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintThemeHelpFormat(void);
