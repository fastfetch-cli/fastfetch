#pragma once

#include "fastfetch.h"

#define FF_LM_MODULE_NAME "LM"

void ffPrintLM(FFLMOptions* options);
void ffInitLMOptions(FFLMOptions* options);
bool ffParseLMCommandOptions(FFLMOptions* options, const char* key, const char* value);
void ffDestroyLMOptions(FFLMOptions* options);
void ffParseLMJsonObject(yyjson_val* module);
