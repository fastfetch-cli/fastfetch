#pragma once

#include "fastfetch.h"

#define FF_WM_MODULE_NAME "WM"

void ffPrintWM(FFWMOptions* options);
void ffInitWMOptions(FFWMOptions* options);
bool ffParseWMCommandOptions(FFWMOptions* options, const char* key, const char* value);
void ffDestroyWMOptions(FFWMOptions* options);
void ffParseWMJsonObject(FFWMOptions* options, yyjson_val* module);
void ffGenerateWMJson(FFWMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintWMHelpFormat(void);
