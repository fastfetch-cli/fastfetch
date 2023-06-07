#pragma once

#include "fastfetch.h"

#define FF_MEDIA_MODULE_NAME "Media"

void ffPrintMedia(FFinstance* instance, FFMediaOptions* options);
void ffInitMediaOptions(FFMediaOptions* options);
bool ffParseMediaCommandOptions(FFMediaOptions* options, const char* key, const char* value);
void ffDestroyMediaOptions(FFMediaOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseMediaJsonObject(FFinstance* instance, json_object* module);
#endif
