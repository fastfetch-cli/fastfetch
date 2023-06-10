
#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_PLAYER_MODULE_NAME "Player"

void ffPrintPlayer(FFinstance* instance, FFPlayerOptions* options);
void ffInitPlayerOptions(FFPlayerOptions* options);
bool ffParsePlayerCommandOptions(FFPlayerOptions* options, const char* key, const char* value);
void ffDestroyPlayerOptions(FFPlayerOptions* options);
void ffParsePlayerJsonObject(FFinstance* instance, yyjson_val* module);
