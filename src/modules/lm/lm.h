#pragma once

#include "option.h"

bool ffPrintLM(FFLMOptions* options);
void ffInitLMOptions(FFLMOptions* options);
void ffDestroyLMOptions(FFLMOptions* options);

extern FFModuleBaseInfo ffLMModuleInfo;
