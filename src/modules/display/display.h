#pragma once

#include "fastfetch.h"

void ffPrintDisplay(FFinstance* instance, FFDisplayOptions* options);
void ffInitDisplayOptions(FFDisplayOptions* options);
bool ffParseDisplayCommandOptions(FFDisplayOptions* options, const char* key, const char* value);
void ffDestroyDisplayOptions(FFDisplayOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
bool ffParseDisplayJsonObject(FFinstance* instance, const char* type, json_object* module);
#endif
