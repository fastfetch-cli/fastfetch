#pragma once

#include "option.h"

#define FF_TITLE_MODULE_NAME "Title"

bool ffPrintTitle(FFTitleOptions* options);
void ffInitTitleOptions(FFTitleOptions* options);
void ffDestroyTitleOptions(FFTitleOptions* options);

extern FFModuleBaseInfo ffTitleModuleInfo;
