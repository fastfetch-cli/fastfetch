#pragma once

#include "option.h"

#define FF_LM_MODULE_NAME "LM"

bool ffPrintLM(FFLMOptions* options);
void ffInitLMOptions(FFLMOptions* options);
void ffDestroyLMOptions(FFLMOptions* options);

extern FFModuleBaseInfo ffLMModuleInfo;
