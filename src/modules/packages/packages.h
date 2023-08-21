#pragma once

#include "fastfetch.h"

#define FF_PACKAGES_MODULE_NAME "Packages"

void ffPrintPackages(FFPackagesOptions* options);
void ffInitPackagesOptions(FFPackagesOptions* options);
bool ffParsePackagesCommandOptions(FFPackagesOptions* options, const char* key, const char* value);
void ffDestroyPackagesOptions(FFPackagesOptions* options);
void ffParsePackagesJsonObject(FFPackagesOptions* options, yyjson_val* module);
