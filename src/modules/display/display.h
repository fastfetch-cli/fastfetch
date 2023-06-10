#pragma once

#include "fastfetch.h"

#define FF_DISPLAY_MODULE_NAME "Display"

void ffPrintDisplay(FFinstance* instance, FFDisplayOptions* options);
void ffInitDisplayOptions(FFDisplayOptions* options);
bool ffParseDisplayCommandOptions(FFDisplayOptions* options, const char* key, const char* value);
void ffDestroyDisplayOptions(FFDisplayOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseDisplayJsonObject(FFinstance* instance, json_object* module);
#endif
