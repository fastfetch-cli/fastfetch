#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_MEDIA_MODULE_NAME "Media"

void ffPrintMedia(FFinstance* instance, FFMediaOptions* options);
void ffInitMediaOptions(FFMediaOptions* options);
bool ffParseMediaCommandOptions(FFMediaOptions* options, const char* key, const char* value);
void ffDestroyMediaOptions(FFMediaOptions* options);
void ffParseMediaJsonObject(FFinstance* instance, yyjson_val* module);
