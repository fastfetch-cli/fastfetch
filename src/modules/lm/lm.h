#pragma once

#include "fastfetch.h"

#define FF_LM_MODULE_NAME "LM"

void ffPrintLM(FFLMOptions* options);
void ffInitLMOptions(FFLMOptions* options);
bool ffParseLMCommandOptions(FFLMOptions* options, const char* key, const char* value);
void ffDestroyLMOptions(FFLMOptions* options);
void ffParseLMJsonObject(FFLMOptions* options, yyjson_val* module);
void ffGenerateLMJsonResult(FFLMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintLMHelpFormat(void);
