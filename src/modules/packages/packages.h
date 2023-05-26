#pragma once

#include "fastfetch.h"
#include "modules/packages/option.h"

#define FF_PACKAGES_MODULE_NAME "Packages"

void ffPrintPackages(FFinstance* instance, FFPackagesOptions* options);
void ffInitPackagesOptions(FFPackagesOptions* options);
bool ffParsePackagesCommandOptions(FFPackagesOptions* options, const char* key, const char* value);
void ffDestroyPackagesOptions(FFPackagesOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParsePackagesJsonObject(FFinstance* instance, json_object* module);
#endif
