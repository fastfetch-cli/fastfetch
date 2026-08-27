
#pragma once

#include "option.h"

bool ffPrintPlayer(FFPlayerOptions* options);
void ffInitPlayerOptions(FFPlayerOptions* options);
void ffDestroyPlayerOptions(FFPlayerOptions* options);

extern FFModuleBaseInfo ffPlayerModuleInfo;
