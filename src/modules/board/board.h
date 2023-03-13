#pragma once

#include "fastfetch.h"

void ffPrintBoard(FFinstance* instance, FFBoardOptions* options);
void ffInitBoardOptions(FFBoardOptions* options);
bool ffParseBoardCommandOptions(FFBoardOptions* options, const char* key, const char* value);
void ffDestroyBoardOptions(FFBoardOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/config.h"
bool ffParseBoardJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module);
#endif
