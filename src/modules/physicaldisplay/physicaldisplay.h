#pragma once

#include "fastfetch.h"

#define FF_PHYSICALDISPLAY_MODULE_NAME "PhysicalDisplay"

void ffPrintPhysicalDisplay(FFPhysicalDisplayOptions* options);
void ffInitPhysicalDisplayOptions(FFPhysicalDisplayOptions* options);
bool ffParsePhysicalDisplayCommandOptions(FFPhysicalDisplayOptions* options, const char* key, const char* value);
void ffDestroyPhysicalDisplayOptions(FFPhysicalDisplayOptions* options);
void ffParsePhysicalDisplayJsonObject(yyjson_val* module);
