#pragma once

#include "fastfetch.h"

#define FF_PHYCIALDISPLAY_MODULE_NAME "PhycialDisplay"

void ffPrintPhycialDisplay(FFPhycialDisplayOptions* options);
void ffInitPhycialDisplayOptions(FFPhycialDisplayOptions* options);
bool ffParsePhycialDisplayCommandOptions(FFPhycialDisplayOptions* options, const char* key, const char* value);
void ffDestroyPhycialDisplayOptions(FFPhycialDisplayOptions* options);
void ffParsePhycialDisplayJsonObject(yyjson_val* module);
