#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_LOCALE_MODULE_NAME "Locale"

void ffPrintLocale(FFinstance* instance, FFLocaleOptions* options);
void ffInitLocaleOptions(FFLocaleOptions* options);
bool ffParseLocaleCommandOptions(FFLocaleOptions* options, const char* key, const char* value);
void ffDestroyLocaleOptions(FFLocaleOptions* options);
void ffParseLocaleJsonObject(FFinstance* instance, yyjson_val* module);
