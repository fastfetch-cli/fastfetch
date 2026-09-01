#pragma once

#include "option.h"

bool ffPrintTitle(FFTitleOptions* options);
void ffInitTitleOptions(FFTitleOptions* options);
void ffDestroyTitleOptions(FFTitleOptions* options);

extern FFModuleBaseInfo ffTitleModuleInfo;
