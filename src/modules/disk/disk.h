#pragma once

#include "option.h"

#define FF_DISK_MODULE_NAME "Disk"

bool ffPrintDisk(FFDiskOptions* options);
void ffInitDiskOptions(FFDiskOptions* options);
void ffDestroyDiskOptions(FFDiskOptions* options);

extern FFModuleBaseInfo ffDiskModuleInfo;
