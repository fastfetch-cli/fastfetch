#pragma once

#include "fastfetch.h"

#define FF_BOOTMGR_MODULE_NAME "Bootmgr"

void ffPrintBootmgr(FFBootmgrOptions* options);
void ffInitBootmgrOptions(FFBootmgrOptions* options);
void ffDestroyBootmgrOptions(FFBootmgrOptions* options);
