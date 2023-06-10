#pragma once

#include "fastfetch.h"

#define FF_CUSTOM_MODULE_NAME "Custom"

void ffPrintCustom(FFinstance* instance, FFCustomOptions* options);
void ffInitCustomOptions(FFCustomOptions* options);
bool ffParseCustomCommandOptions(FFCustomOptions* options, const char* key, const char* value);
void ffDestroyCustomOptions(FFCustomOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseCustomJsonObject(FFinstance* instance, json_object* module);
#endif
