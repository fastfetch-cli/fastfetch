#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_DISPLAY_MODULE_NAME "Display"

void ffPrintDisplay(FFinstance* instance, FFDisplayOptions* options);
void ffInitDisplayOptions(FFDisplayOptions* options);
bool ffParseDisplayCommandOptions(FFDisplayOptions* options, const char* key, const char* value);
void ffDestroyDisplayOptions(FFDisplayOptions* options);
void ffParseDisplayJsonObject(FFinstance* instance, yyjson_val* module);
