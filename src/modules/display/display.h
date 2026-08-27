#pragma once

#include "option.h"

bool ffPrintDisplay(FFDisplayOptions* options);
void ffInitDisplayOptions(FFDisplayOptions* options);
void ffDestroyDisplayOptions(FFDisplayOptions* options);

extern FFModuleBaseInfo ffDisplayModuleInfo;
