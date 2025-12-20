#pragma once

#include "option.h"

#define FF_DISPLAY_MODULE_NAME "Display"

bool ffPrintDisplay(FFDisplayOptions* options);
void ffInitDisplayOptions(FFDisplayOptions* options);
void ffDestroyDisplayOptions(FFDisplayOptions* options);

extern FFModuleBaseInfo ffDisplayModuleInfo;
