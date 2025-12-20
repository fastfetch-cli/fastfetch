#pragma once

#include "option.h"

#define FF_PHYSICALDISK_MODULE_NAME "PhysicalDisk"

bool ffPrintPhysicalDisk(FFPhysicalDiskOptions* options);
void ffInitPhysicalDiskOptions(FFPhysicalDiskOptions* options);
void ffDestroyPhysicalDiskOptions(FFPhysicalDiskOptions* options);

extern FFModuleBaseInfo ffPhysicalDiskModuleInfo;
