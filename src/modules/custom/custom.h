#pragma once

#include "option.h"

#define FF_CUSTOM_MODULE_NAME "Custom"

bool ffPrintCustom(FFCustomOptions* options);
void ffInitCustomOptions(FFCustomOptions* options);
void ffDestroyCustomOptions(FFCustomOptions* options);

extern FFModuleBaseInfo ffCustomModuleInfo;
