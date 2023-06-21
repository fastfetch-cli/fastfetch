#pragma once

#include "fastfetch.h"

#define FF_LM_MODULE_NAME "LM"

void ffPrintLM(FFinstance* instance, FFLMOptions* options);
void ffInitLMOptions(FFLMOptions* options);
bool ffParseLMCommandOptions(FFLMOptions* options, const char* key, const char* value);
void ffDestroyLMOptions(FFLMOptions* options);
void ffParseLMJsonObject(FFinstance* instance, yyjson_val* module);
