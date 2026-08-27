#pragma once

#include "option.h"

bool ffPrintDisk(FFDiskOptions* options);
void ffInitDiskOptions(FFDiskOptions* options);
void ffDestroyDiskOptions(FFDiskOptions* options);

extern FFModuleBaseInfo ffDiskModuleInfo;
