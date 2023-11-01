#pragma once

#include "fastfetch.h"

#define FF_DISK_MODULE_NAME "Disk"

void ffPrintDisk(FFDiskOptions* options);
void ffInitDiskOptions(FFDiskOptions* options);
void ffDestroyDiskOptions(FFDiskOptions* options);
