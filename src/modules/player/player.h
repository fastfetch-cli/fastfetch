
#pragma once

#include "fastfetch.h"

#define FF_PLAYER_MODULE_NAME "Player"

void ffPrintPlayer(FFinstance* instance, FFPlayerOptions* options);
void ffInitPlayerOptions(FFPlayerOptions* options);
bool ffParsePlayerCommandOptions(FFPlayerOptions* options, const char* key, const char* value);
void ffDestroyPlayerOptions(FFPlayerOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParsePlayerJsonObject(FFinstance* instance, json_object* module);
#endif
