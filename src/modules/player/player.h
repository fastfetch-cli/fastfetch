
#pragma once

#include "fastfetch.h"

#define FF_PLAYER_MODULE_NAME "Player"

void ffPrintPlayer(FFPlayerOptions* options);
void ffInitPlayerOptions(FFPlayerOptions* options);
bool ffParsePlayerCommandOptions(FFPlayerOptions* options, const char* key, const char* value);
void ffDestroyPlayerOptions(FFPlayerOptions* options);
void ffParsePlayerJsonObject(FFPlayerOptions* options, yyjson_val* module);
