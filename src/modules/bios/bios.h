#pragma once

#include "option.h"

bool ffPrintBios(FFBiosOptions* options);
void ffInitBiosOptions(FFBiosOptions* options);
void ffDestroyBiosOptions(FFBiosOptions* options);

extern FFModuleBaseInfo ffBiosModuleInfo;
