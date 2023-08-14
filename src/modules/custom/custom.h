#pragma once

#include "fastfetch.h"

#define FF_CUSTOM_MODULE_NAME "Custom"

void ffPrintCustom(FFCustomOptions* options);
void ffInitCustomOptions(FFCustomOptions* options);
bool ffParseCustomCommandOptions(FFCustomOptions* options, const char* key, const char* value);
void ffDestroyCustomOptions(FFCustomOptions* options);
void ffParseCustomJsonObject(yyjson_val* module);
