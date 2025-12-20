#pragma once

#include "option.h"

#define FF_BIOS_MODULE_NAME "BIOS"

bool ffPrintBios(FFBiosOptions* options);
void ffInitBiosOptions(FFBiosOptions* options);
void ffDestroyBiosOptions(FFBiosOptions* options);

extern FFModuleBaseInfo ffBiosModuleInfo;
