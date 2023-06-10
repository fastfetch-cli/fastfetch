#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_CUSTOM_MODULE_NAME "Custom"

void ffPrintCustom(FFinstance* instance, FFCustomOptions* options);
void ffInitCustomOptions(FFCustomOptions* options);
bool ffParseCustomCommandOptions(FFCustomOptions* options, const char* key, const char* value);
void ffDestroyCustomOptions(FFCustomOptions* options);
void ffParseCustomJsonObject(FFinstance* instance, yyjson_val* module);
