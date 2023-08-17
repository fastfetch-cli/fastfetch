#pragma once

#include "fastfetch.h"

#define FF_LOCALE_MODULE_NAME "Locale"

void ffPrintLocale(FFLocaleOptions* options);
void ffInitLocaleOptions(FFLocaleOptions* options);
bool ffParseLocaleCommandOptions(FFLocaleOptions* options, const char* key, const char* value);
void ffDestroyLocaleOptions(FFLocaleOptions* options);
void ffParseLocaleJsonObject(FFLocaleOptions* options, yyjson_val* module);
