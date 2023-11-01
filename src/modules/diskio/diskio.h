#pragma once

#include "fastfetch.h"

#define FF_DISKIO_MODULE_NAME "DiskIO"

void ffPrepareDiskIO(FFDiskIOOptions* options);

void ffPrintDiskIO(FFDiskIOOptions* options);
void ffInitDiskIOOptions(FFDiskIOOptions* options);
void ffDestroyDiskIOOptions(FFDiskIOOptions* options);
