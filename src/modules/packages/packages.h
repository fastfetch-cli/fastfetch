#pragma once

#include "option.h"

bool ffPrintPackages(FFPackagesOptions* options);
void ffInitPackagesOptions(FFPackagesOptions* options);
void ffDestroyPackagesOptions(FFPackagesOptions* options);

extern FFModuleBaseInfo ffPackagesModuleInfo;
