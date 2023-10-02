#pragma once

#include "fastfetch.h"

#define FF_MEDIA_MODULE_NAME "Media"

void ffPrintMedia(FFMediaOptions* options);
void ffInitMediaOptions(FFMediaOptions* options);
bool ffParseMediaCommandOptions(FFMediaOptions* options, const char* key, const char* value);
void ffDestroyMediaOptions(FFMediaOptions* options);
void ffParseMediaJsonObject(FFMediaOptions* options, yyjson_val* module);
void ffGenerateMediaJson(FFMediaOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
