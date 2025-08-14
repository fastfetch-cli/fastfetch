
#pragma once

#include "option.h"

#define FF_PLAYER_MODULE_NAME "Player"

void ffPrintPlayer(FFPlayerOptions* options);
void ffInitPlayerOptions(FFPlayerOptions* options);
void ffDestroyPlayerOptions(FFPlayerOptions* options);

extern FFModuleBaseInfo ffPlayerModuleInfo;
