#pragma once

#include "option.h"

bool ffPrintDE(FFDEOptions* options);
void ffInitDEOptions(FFDEOptions* options);
void ffDestroyDEOptions(FFDEOptions* options);

extern FFModuleBaseInfo ffDEModuleInfo;
