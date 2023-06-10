#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_PACKAGES_MODULE_NAME "Packages"

void ffPrintPackages(FFinstance* instance, FFPackagesOptions* options);
void ffInitPackagesOptions(FFPackagesOptions* options);
bool ffParsePackagesCommandOptions(FFPackagesOptions* options, const char* key, const char* value);
void ffDestroyPackagesOptions(FFPackagesOptions* options);
void ffParsePackagesJsonObject(FFinstance* instance, yyjson_val* module);
