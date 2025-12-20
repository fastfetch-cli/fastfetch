#pragma once

#include "option.h"

#define FF_DE_MODULE_NAME "DE"

bool ffPrintDE(FFDEOptions* options);
void ffInitDEOptions(FFDEOptions* options);
void ffDestroyDEOptions(FFDEOptions* options);

extern FFModuleBaseInfo ffDEModuleInfo;
