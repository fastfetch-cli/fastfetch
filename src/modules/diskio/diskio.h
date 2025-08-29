#pragma once

#include "option.h"

#define FF_DISKIO_MODULE_NAME "DiskIO"

void ffPrepareDiskIO(FFDiskIOOptions* options);

bool ffPrintDiskIO(FFDiskIOOptions* options);
void ffInitDiskIOOptions(FFDiskIOOptions* options);
void ffDestroyDiskIOOptions(FFDiskIOOptions* options);

extern FFModuleBaseInfo ffDiskIOModuleInfo;
