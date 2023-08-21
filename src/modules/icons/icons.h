#pragma once

#include "fastfetch.h"

#define FF_ICONS_MODULE_NAME "Icons"

void ffPrintIcons(FFIconsOptions* options);
void ffInitIconsOptions(FFIconsOptions* options);
bool ffParseIconsCommandOptions(FFIconsOptions* options, const char* key, const char* value);
void ffDestroyIconsOptions(FFIconsOptions* options);
void ffParseIconsJsonObject(FFIconsOptions* options, yyjson_val* module);
