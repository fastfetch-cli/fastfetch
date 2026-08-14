#pragma once

#include "option.h"

bool ffPrintBootmgr(FFBootmgrOptions* options);
void ffInitBootmgrOptions(FFBootmgrOptions* options);
void ffDestroyBootmgrOptions(FFBootmgrOptions* options);

extern FFModuleBaseInfo ffBootmgrModuleInfo;
