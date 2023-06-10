#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_SWAP_MODULE_NAME "Swap"

void ffPrintSwap(FFinstance* instance, FFSwapOptions* options);
void ffInitSwapOptions(FFSwapOptions* options);
bool ffParseSwapCommandOptions(FFSwapOptions* options, const char* key, const char* value);
void ffDestroySwapOptions(FFSwapOptions* options);
void ffParseSwapJsonObject(FFinstance* instance, yyjson_val* module);
