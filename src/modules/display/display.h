#pragma once

#include "fastfetch.h"

#define FF_DISPLAY_MODULE_NAME "Display"

void ffPrintDisplay(FFDisplayOptions* options);
void ffInitDisplayOptions(FFDisplayOptions* options);
bool ffParseDisplayCommandOptions(FFDisplayOptions* options, const char* key, const char* value);
void ffDestroyDisplayOptions(FFDisplayOptions* options);
void ffParseDisplayJsonObject(FFDisplayOptions* options, yyjson_val* module);
