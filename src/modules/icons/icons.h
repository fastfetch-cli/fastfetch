#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_ICONS_MODULE_NAME "Icons"

void ffPrintIcons(FFinstance* instance, FFIconsOptions* options);
void ffInitIconsOptions(FFIconsOptions* options);
bool ffParseIconsCommandOptions(FFIconsOptions* options, const char* key, const char* value);
void ffDestroyIconsOptions(FFIconsOptions* options);
void ffParseIconsJsonObject(FFinstance* instance, yyjson_val* module);
