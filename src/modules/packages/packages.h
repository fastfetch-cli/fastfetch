#pragma once

#include "fastfetch.h"

#define FF_PACKAGES_MODULE_NAME "Packages"

void ffPrintPackages(FFPackagesOptions* options);
void ffInitPackagesOptions(FFPackagesOptions* options);
bool ffParsePackagesCommandOptions(FFPackagesOptions* options, const char* key, const char* value);
void ffDestroyPackagesOptions(FFPackagesOptions* options);
void ffParsePackagesJsonObject(FFPackagesOptions* options, yyjson_val* module);
void ffGeneratePackagesJson(FFPackagesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintPackagesHelpFormat(void);
