#pragma once

#include "option.h"

void ffPrepareDiskIO(FFDiskIOOptions* options);

bool ffPrintDiskIO(FFDiskIOOptions* options);
void ffInitDiskIOOptions(FFDiskIOOptions* options);
void ffDestroyDiskIOOptions(FFDiskIOOptions* options);

extern FFModuleBaseInfo ffDiskIOModuleInfo;
