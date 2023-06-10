#pragma once

#include "fastfetch.h"

#define FF_WM_MODULE_NAME "WM"

void ffPrintWM(FFinstance* instance, FFWMOptions* options);
void ffInitWMOptions(FFWMOptions* options);
bool ffParseWMCommandOptions(FFWMOptions* options, const char* key, const char* value);
void ffDestroyWMOptions(FFWMOptions* options);
void ffParseWMJsonObject(FFinstance* instance, yyjson_val* module);
