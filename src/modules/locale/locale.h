#pragma once

#include "fastfetch.h"

#define FF_LOCALE_MODULE_NAME "Locale"

void ffPrintLocale(FFinstance* instance, FFLocaleOptions* options);
void ffInitLocaleOptions(FFLocaleOptions* options);
bool ffParseLocaleCommandOptions(FFLocaleOptions* options, const char* key, const char* value);
void ffDestroyLocaleOptions(FFLocaleOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseLocaleJsonObject(FFinstance* instance, json_object* module);
#endif
