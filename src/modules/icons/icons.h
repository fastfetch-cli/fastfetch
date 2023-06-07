#pragma once

#include "fastfetch.h"

#define FF_ICONS_MODULE_NAME "Icons"

void ffPrintIcons(FFinstance* instance, FFIconsOptions* options);
void ffInitIconsOptions(FFIconsOptions* options);
bool ffParseIconsCommandOptions(FFIconsOptions* options, const char* key, const char* value);
void ffDestroyIconsOptions(FFIconsOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseIconsJsonObject(FFinstance* instance, json_object* module);
#endif
