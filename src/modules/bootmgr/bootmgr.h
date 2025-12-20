#pragma once

#include "option.h"

#define FF_BOOTMGR_MODULE_NAME "Bootmgr"

bool ffPrintBootmgr(FFBootmgrOptions* options);
void ffInitBootmgrOptions(FFBootmgrOptions* options);
void ffDestroyBootmgrOptions(FFBootmgrOptions* options);

extern FFModuleBaseInfo ffBootmgrModuleInfo;
