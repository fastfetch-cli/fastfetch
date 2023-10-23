#pragma once

#include "fastfetch.h"

#define FF_PACKAGES_MODULE_NAME "Packages"

void ffPrintPackages(FFPackagesOptions* options);
void ffInitPackagesOptions(FFPackagesOptions* options);
void ffDestroyPackagesOptions(FFPackagesOptions* options);
