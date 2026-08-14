#pragma once

#include "option.h"

bool ffPrintCustom(FFCustomOptions* options);
void ffInitCustomOptions(FFCustomOptions* options);
void ffDestroyCustomOptions(FFCustomOptions* options);

extern FFModuleBaseInfo ffCustomModuleInfo;
