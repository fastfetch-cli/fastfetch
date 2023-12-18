#pragma once

#include "fastfetch.h"

#define FF_PHYSICALDISK_MODULE_NAME "PhysicalDisk"

void ffPrintPhysicalDisk(FFPhysicalDiskOptions* options);
void ffInitPhysicalDiskOptions(FFPhysicalDiskOptions* options);
void ffDestroyPhysicalDiskOptions(FFPhysicalDiskOptions* options);
