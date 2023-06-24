#pragma once

#include "fastfetch.h"

#define FF_MEDIA_MODULE_NAME "Media"

void ffPrintMedia(FFMediaOptions* options);
void ffInitMediaOptions(FFMediaOptions* options);
bool ffParseMediaCommandOptions(FFMediaOptions* options, const char* key, const char* value);
void ffDestroyMediaOptions(FFMediaOptions* options);
void ffParseMediaJsonObject(yyjson_val* module);
