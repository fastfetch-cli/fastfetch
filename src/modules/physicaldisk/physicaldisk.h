#pragma once

#include "option.h"

bool ffPrintPhysicalDisk(FFPhysicalDiskOptions* options);
void ffInitPhysicalDiskOptions(FFPhysicalDiskOptions* options);
void ffDestroyPhysicalDiskOptions(FFPhysicalDiskOptions* options);

extern FFModuleBaseInfo ffPhysicalDiskModuleInfo;
