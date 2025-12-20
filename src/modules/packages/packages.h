#pragma once

#include "option.h"

#define FF_PACKAGES_MODULE_NAME "Packages"

bool ffPrintPackages(FFPackagesOptions* options);
void ffInitPackagesOptions(FFPackagesOptions* options);
void ffDestroyPackagesOptions(FFPackagesOptions* options);

extern FFModuleBaseInfo ffPackagesModuleInfo;
